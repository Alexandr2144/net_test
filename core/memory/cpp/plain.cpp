#include "core/memory/plain.h"
#include "core/memory/common.h"
#include "core/math/common.h"

#include <memory.h>


namespace Memory {
    // ------------------------------------------------------------------------
    // Chain implementation
    // ------------------------------------------------------------------------
    Chain::Chain(size_t startsize, BuddyHeap* heap)
        : m_nextsize(getbuddysize(getbuddylevel(startsize == 0 ? 1 : startsize)))
        , m_heap(heap ? *heap : buddy_global_heap)
        , lastChunk(nullptr)
    {
        extend(startsize, true);
    }

    // ------------------------------------------------------------------------
    Chain::Chain(Chain&& chain)
        : m_nextsize(chain.m_nextsize)
        , m_heap(chain.m_heap)
        , lastChunk(chain.lastChunk)
    {
        chain.lastChunk = nullptr;
        chain.m_nextsize = 0;
    }

    // ------------------------------------------------------------------------
    Chain& Chain::operator=(Chain&& chain)
    {
        dealloc();
        new(this) Chain(std::forward<Chain>(chain));
        return *this;
    }

    // ------------------------------------------------------------------------
    Chain::~Chain()
    {
        dealloc();
    }

    // ------------------------------------------------------------------------
    void Chain::clear(size_t startsize)
    {
        dealloc();
        m_nextsize = getbuddysize(getbuddylevel(startsize == 0 ? 1 : startsize));
        extend(startsize, true);
    }

    // ------------------------------------------------------------------------
    void Chain::dealloc()
    {
        Chunk* chunk = lastChunk;
        while (chunk) {
            lastChunk = chunk->prev;
            m_heap.dealloc(chunk);
            chunk = lastChunk;
        }
    }

    // ------------------------------------------------------------------------
    void Chain::back(size_t count)
    {
        size_t tail = lastChunk->size();
        while (count > tail) {
            count -= tail;
            lastChunk->clear();
            lastChunk = lastChunk->prev;

            tail = lastChunk->size();
        }
        lastChunk->last -= count;
    }

    // ------------------------------------------------------------------------
    Bytes Chain::forward(size_t count)
    {
        while (lastChunk->size() == 0) {
            if (!lastChunk->next) {
                extend(0, false);
                break;
            }
            lastChunk = lastChunk->next;
        }

        size_t length = Math::min(lastChunk->size(), count);

        Byte* begin = lastChunk->last;
        lastChunk->last += length;

        return Bytes{ begin, lastChunk->last };
    }

    // ------------------------------------------------------------------------
    Bytes Chain::reserve(size_t count)
    {
        while (!has_space(count)) {
            if (!lastChunk->next) {
                extend(count, false);
                break;
            }
            lastChunk = lastChunk->next;
        }

        Byte* begin = lastChunk->last;
        lastChunk->last += count;

        return Bytes{ begin, lastChunk->last };
    }

    // ------------------------------------------------------------------------
    bool Chain::has_space(size_t count)
    {
        if (lastChunk == nullptr) {
            return false;
        }
        return (lastChunk->last + count <= lastChunk->end);
    }

    // ------------------------------------------------------------------------
    void Chain::extend(size_t count, bool init)
    {
        size_t minSize = count + sizeof(Chunk);
        Bytes memory = m_heap.alloc(Math::max(m_nextsize, minSize));

        Chunk* chunk = (Chunk*)memory.begin;
        chunk->last = memory.begin + sizeof(Chunk);
        chunk->end = memory.end;

        chunk->next = nullptr;
        chunk->prev = lastChunk;

        if (!init) {
            M_ASSERT(lastChunk != nullptr);
            lastChunk->next = chunk;
        }

        lastChunk = chunk;
        m_nextsize <<= 1;
    }

    // ------------------------------------------------------------------------
    // ChainBuffer implementation
    // ------------------------------------------------------------------------
    ChainBuffer::ChainBuffer(size_t startsize, BuddyHeap* heap)
        : Chain(startsize == 0 ? 32 : startsize, heap)
    {
        m_firstChunk = lastChunk;
    }

    // ------------------------------------------------------------------------
    void ChainBuffer::clear(size_t startsize)
    {
        Chain::clear(startsize);
        m_firstChunk = lastChunk;
        m_totalSize = 0;
    }

    // ------------------------------------------------------------------------
    void ChainBuffer::extract(Bytes dest) const
    {
        M_ASSERT(count(dest) == m_totalSize);

        for (Chain::Chunk* chunk = m_firstChunk; chunk; chunk = chunk->next) {
            Bytes src{ (Byte*)(chunk + 1), chunk->last };

            Bytes forWriting = Data::split<Byte>(&dest, Data::count(src));
            Data::bytecopy(src, forWriting);
        }
    }

    // ------------------------------------------------------------------------
    void ChainBuffer::extract(Byte* dest) const
    {
        for (Chain::Chunk* chunk = m_firstChunk; chunk; chunk = chunk->next) {
            Byte* src = (Byte*)(chunk + 1);
            size_t amount = chunk->last - src;

            memcpy(dest, src, amount);
            dest += amount;
        }
    }

    // ------------------------------------------------------------------------
    // Region implementation
    // ------------------------------------------------------------------------
    Region::Region(size_t startsize, BuddyHeap* heap)
        : m_heap(heap ? *heap : buddy_global_heap)
    {
        reserve(startsize);
    }

    // ------------------------------------------------------------------------
    Region::~Region()
    {
        m_heap.dealloc(memory.begin);
    }

    // ------------------------------------------------------------------------
    Region::Region(Region&& reg)
        : memory(reg.memory)
        , m_heap(reg.m_heap)
    {
		reg.memory = Bytes();
    }

    // ------------------------------------------------------------------------
    Region& Region::operator=(Region&& reg)
    {
        m_heap.dealloc(memory.begin);
        new(this) Region(std::forward<Region>(reg));
        return *this;
    }

    // ------------------------------------------------------------------------
    void* Region::get(size_t idx, size_t size) const
    {
        return memory.begin + (idx * size);
    }

    // ------------------------------------------------------------------------
    Bytes Region::reserve(size_t offset)
    {
        size_t size = Data::count(memory);

        if (offset > size) {
            memory = m_heap.realloc(memory.begin, offset);
        }
        M_ASSERT(offset <= Data::count(memory));
        return Bytes{ memory.begin, memory.begin + offset };
    }

    // ------------------------------------------------------------------------
    void Region::clear()
    {
       m_heap.dealloc(memory.begin);
       memory = Bytes();
    }

    // ------------------------------------------------------------------------
    // RegBuffer implementation
    // ------------------------------------------------------------------------
    RegBuffer::RegBuffer(size_t startsize, BuddyHeap* heap)
        : Region(startsize, heap)
        , m_totalSize(0)
    {
    }

    // ------------------------------------------------------------------------
    RegBuffer::RegBuffer(RegBuffer&& buf)
        : Region(std::forward<Region>(buf))
        , m_totalSize(buf.m_totalSize)
    {
        buf.m_totalSize = 0;
    }

    // ------------------------------------------------------------------------
    RegBuffer& RegBuffer::operator=(RegBuffer&& buf)
    {
        new(this) RegBuffer(std::forward<RegBuffer>(buf));
        return *this;
    }

    // ------------------------------------------------------------------------
    Bytes RegBuffer::reserve(size_t count)
    {
        Region::reserve(m_totalSize + count);
        Bytes mem{ memory.begin + m_totalSize, memory.begin + m_totalSize + count };

        m_totalSize += count;
        return mem;
    }

    // ------------------------------------------------------------------------
    void RegBuffer::extract(Bytes const& dest) const
    {
        bytecopy(dest, Bytes{ memory.begin, memory.begin + m_totalSize });
    }

    // ------------------------------------------------------------------------
    void RegBuffer::extract(Byte* dest) const
    {
        memcpy(dest, memory.begin, m_totalSize);
    }

    // ------------------------------------------------------------------------
    // RegBuffer implementation
    // ------------------------------------------------------------------------
    Pool::Pool(size_t startsize, BuddyHeap* heap)
        : m_chain(startsize, heap)
        , m_first(nullptr)
    {
    }

    // ------------------------------------------------------------------------
    Pool::Pool(Pool&& pool)
        : m_chain(std::forward<Chain>(pool.m_chain))
        , m_first(pool.m_first)
    {
        pool.m_first = nullptr;
    }

    // ------------------------------------------------------------------------
    Pool& Pool::operator=(Pool&& pool)
    {
        new(this) Pool(std::forward<Pool>(pool));
        return *this;
    }

    // ------------------------------------------------------------------------
    void Pool::clear(size_t startsize)
    {
        m_chain.clear(startsize);
        m_first = nullptr;
    }

    // ------------------------------------------------------------------------
    void Pool::reset(size_t cellSize)
    {
        m_first = nullptr;

        Chain::Chunk* chunk = m_chain.lastChunk;
        while (chunk) {
            reset_chunk(chunk, cellSize);
            chunk = chunk->prev;
        }
    }

    // ------------------------------------------------------------------------
    void Pool::reset_chunk(Chain::Chunk* chunk, size_t cellSize)
    {
        Byte* ptr = (Byte*)(chunk + 1);
        while (ptr < chunk->last) {
            dealloc(ptr);
            ptr += cellSize;
        }
    }

    // ------------------------------------------------------------------------
    void* Pool::alloc(size_t size)
    {
        if (m_first) {
            FreeCell* cell = m_first;
            m_first = m_first->next;
            return cell;
        }
        else {
            Bytes bytes = m_chain.reserve(size);
            return bytes.begin;
        }
    }

    // ------------------------------------------------------------------------
    void Pool::dealloc(void* ptr)
    {
        FreeCell* cell = (FreeCell*)ptr;
        cell->next = m_first;
        m_first = cell;
    }

} // namespace Memory