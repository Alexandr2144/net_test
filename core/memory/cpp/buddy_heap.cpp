#include "core/memory/buddy_heap.h"
#include "core/math/common.h"
#include "native/memory.h"

#include <malloc.h>
#include <memory.h>


#define M_BUDDY_STRATEGY M_BUDDY_STRATEGY_MALLOC

#if (M_BUDDY_STRATEGY == M_BUDDY_STRATEGY_MALLOC)
#  include <malloc.h>
#  define M_BUDDY_ALLOC_STD   malloc
#  define M_BUDDY_REALLOC_STD ::realloc
#  define M_BUDDY_DEALLOC_STD free
#elif  (M_BUDDY_STRATEGY == M_BUDDY_STRATEGY_DLMALLOC)
#  include "memory/dlmalloc.h"
#  define M_BUDDY_ALLOC_STD   ::dlmalloc
#  define M_BUDDY_REALLOC_STD ::dlrealloc
#  define M_BUDDY_DEALLOC_STD ::dlfree
#endif // M_BUDDY_STRATEGY


namespace Memory {
    using Chunk = BuddyArena_ByNode::Chunk;
    using Block = BuddyArena_ByNode::Block;

	using Data::Ref;


	BuddyHeap buddy_global_heap;


    // ------------------------------------------------------------------------
    size_t getbuddylevel(size_t size)
    {
        static const size_t base = Math::log2(BuddyArena::MIN_SIZE);
        size_t power = Math::log2(int(2 * size - 1));
        return power - base;
    }

    // ------------------------------------------------------------------------
    size_t getbuddysize(size_t level)
    {
        static const size_t base = Math::log2(BuddyArena::MIN_SIZE);
        return size_t(1) << (level + base);
    }

	// ------------------------------------------------------------------------
	// BuddyArena_ByNode implementation
    // ------------------------------------------------------------------------
    static void allocNewChunk(Ref<BuddyArena_ByNode> arena)
    {
        static const size_t CHUNKSIZE = BuddyArena_ByNode::MIN_SIZE * (1 << BuddyArena_ByNode::MAX_BINS);
        static const size_t CNKINF_SIZE = (sizeof(Chunk) + 31) & ~31;

        uintptr_t rawptr = (uintptr_t)malloc(2 * CNKINF_SIZE);
        Chunk* chunk = (Chunk*)(rawptr + 31 & ~31);
        M_ASSERT(((uintptr_t)chunk & 0x1F) == 0);

        chunk->mem = Native::mcommit(nullptr, CHUNKSIZE);
        chunk->end = (char*)chunk->mem + CHUNKSIZE;

        Chunk* chunksHead = arena.ref.chunks;
        chunk->prev = nullptr;
        chunk->next = chunksHead;

        if (chunksHead) chunksHead->prev = chunk;
		arena.ref.chunks = chunk;

        Block* block = (Block*)chunk->mem;
        block->prev = block;
        block->next = block;

        block->handle = (uintptr_t)chunk | (BuddyArena_ByNode::MAX_BINS - 1);

		arena.ref.bins[BuddyArena_ByNode::MAX_BINS - 1] = block;
    }

    // ------------------------------------------------------------------------
	BuddyArena_ByNode::BuddyArena_ByNode()
    {
        memset(this, 0x00, sizeof(*this));
        allocNewChunk(this);
    }

    // ------------------------------------------------------------------------
	BuddyArena_ByNode::~BuddyArena_ByNode()
    {
    }

    // ------------------------------------------------------------------------
    Bytes Block::freespace(Block* block, size_t size)
    {
        return Bytes{ (Byte*)block + HEADER_SIZE, (Byte*)block + size };
    }

    // ------------------------------------------------------------------------
    Block* Block::fromptr(void* ptr)
    {
        return (Block*)((Byte*)ptr - HEADER_SIZE);
    }

    // ------------------------------------------------------------------------
    void Block::setattr(size_t lvl, size_t bits)
    {
        handle &= ~0x1F;
        handle |= lvl | bits;
    }

    // ------------------------------------------------------------------------
    bool   Block::isfree() const { return (handle & USED_BIT) == 0; }
    Chunk* Block::owner()  const { return (Chunk*)(handle & ~0x1F); }
    size_t Block::level()  const { return size_t(handle & 0x0F); }
    
    // ------------------------------------------------------------------------
    inline static size_t getFreeLevel(BuddyArena_ByNode const& arena, size_t level)
    {
        while (level != BuddyArena_ByNode::MAX_BINS) {
            if (arena.bins[level]) break;
            level += 1;
        }
        return level;
    }

    // ------------------------------------------------------------------------
    inline static Block* getBuddyBlock(Chunk* chunk, Block* block, size_t size)
    {
        Byte* baseptr = (Byte*)chunk->mem;
        uintptr_t delta = (Byte*)block - baseptr;
        uintptr_t buddy = delta ^ size;

        return (Block*)(baseptr + buddy);
    }

    // ------------------------------------------------------------------------
    inline static void removeFreeBlock(Ref<BuddyArena_ByNode> arena, Block* block, size_t level)
    {
        block->prev->next = block->next;
        block->next->prev = block->prev;

        if (block->prev != block->next) {
			arena.ref.bins[level] = block->prev;
        }
        else {
			arena.ref.bins[level] = nullptr;
        }
    }

    // ------------------------------------------------------------------------
    inline static void addFreeBlock(Ref<BuddyArena_ByNode> arena, Block* block, size_t level)
    {
        Block* blockHead = arena.ref.bins[level];
        if (blockHead) {
            block->prev = blockHead->prev;
            block->next = blockHead;

            blockHead->prev->next = block;
            blockHead->prev = block;
        }
        else {
			arena.ref.bins[level] = block;
            block->prev = block;
            block->next = block;
        }
    }

    // ------------------------------------------------------------------------
    Bytes split(Ref<BuddyArena_ByNode> arena, Block* block, size_t splitlevel, size_t level)
    {
        Chunk* chunk = (Chunk*)(block->handle & ~0x1F);
        size_t splitsize = getbuddysize(splitlevel);

        while (splitlevel != level) {
            splitlevel -= 1;
            splitsize >>= 1;

            Block* buddy = getBuddyBlock(chunk, block, splitsize);

            addFreeBlock(arena, buddy, splitlevel);
            buddy->handle = block->handle;
            buddy->setattr(splitlevel, 0);
        }

        block->setattr(level, Block::USED_BIT);
        return Block::freespace(block, splitsize);
    }

    // ------------------------------------------------------------------------
    bool merge(Ref<BuddyArena_ByNode> arena, Block* block, Block* buddy, size_t level)
    {
        size_t buddyInf = buddy->handle & 0x1F;
        if (buddyInf == level) {
            Block* merged = Math::min(buddy, block);
            removeFreeBlock(&arena, buddy, level);
            merged->setattr(level + 1, 0);
            return true;
        }
        else {
            block->setattr(level, 0);
            return false;
        }
    }

    // ------------------------------------------------------------------------
    Bytes BuddyArena_ByNode::realloc(void* ptr, size_t minsize)
    {
        if (ptr != nullptr) {
            dealloc(ptr);
        }
        return alloc(minsize);
    }

    // ------------------------------------------------------------------------
    Bytes BuddyArena_ByNode::alloc(size_t minsize)
    {
		size_t level = getbuddylevel(minsize);

        if (level >= BuddyArena_ByNode::MAX_BINS)
            Native::assert_fail(M_CL, "Unmanaged allocation not realized yet");

        size_t freeLevel = getFreeLevel(*this, level);
        if (freeLevel == BuddyArena_ByNode::MAX_BINS) allocNewChunk(this);

        Block* block = bins[freeLevel];
        M_ASSERT(block != nullptr);

        if (freeLevel == level) {
            removeFreeBlock(this, block, level);
            block->setattr(level, Block::USED_BIT);

            return Block::freespace(block, getbuddysize(level));
        }
        else {
            return split(this, block, freeLevel, level);
        }
    }

    // ------------------------------------------------------------------------
    void BuddyArena_ByNode::dealloc(void* ptr)
    {
        static const size_t MAX_SIZE = getbuddysize(15) - Block::HEADER_SIZE;

        Block* block = Block::fromptr(ptr);
        Chunk* chunk = block->owner();

        size_t level = block->level();
        size_t size = getbuddysize(level);
        while (level != BuddyArena_ByNode::MAX_BINS - 1) {
            Block* buddy = getBuddyBlock(chunk, block, size);
            if (!merge(this, block, buddy, level)) break;

            block = Math::min(block, buddy);
            level += 1;
            size <<= 1;
        }

        // if (level == BuddyAllocator::MAX_BINS) freeChunk(block)

        addFreeBlock(this, block, level);
    }

	// ------------------------------------------------------------------------
	// BuddyHeap implementation
	// ------------------------------------------------------------------------
	Bytes BuddyHeap::alloc(size_t minsize)
	{
#ifdef M_BUDDY_ALLOC_STD
		if (this == &buddy_global_heap) {
			Byte* memory = (Byte*)M_BUDDY_ALLOC_STD(minsize);
			return Bytes{ memory, memory + minsize };
		}
#endif
		return m_arena.alloc(minsize);
	}

    // ------------------------------------------------------------------------
    Bytes BuddyHeap::realloc(void* ptr, size_t minsize)
    {
#ifdef M_BUDDY_ALLOC_STD
        if (this == &buddy_global_heap) {
            Byte* memory = (Byte*)M_BUDDY_REALLOC_STD(ptr, minsize);
            return Bytes{ memory, memory + minsize };
        }
#endif
        return m_arena.realloc(ptr, minsize);
    }

	// ------------------------------------------------------------------------
	void BuddyHeap::dealloc(void* ptr)
	{
#ifdef M_BUDDY_DEALLOC_STD
		if (this == &buddy_global_heap) {
			M_BUDDY_DEALLOC_STD(ptr);
			return;
		}
#endif
		m_arena.dealloc(ptr);
	}

} // namespace Memory