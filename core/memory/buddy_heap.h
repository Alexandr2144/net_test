#pragma once
#ifndef MEMORY_BUDDY_HEAP_H
#define MEMORY_BUDDY_HEAP_H

#include "core/data/array.h"
#include "core/data/pointers.h"
#include "core/memory/common.h"


#define M_BUDDY_STRATEGY_BYMASK   0
#define M_BUDDY_STRATEGY_BYNODE   1
#define M_BUDDY_STRATEGY_MALLOC   2
#define M_BUDDY_STRATEGY_DLMALLOC 3

#ifndef M_BUDDY_STRATEGY
#  define M_BUDDY_STRATEGY M_BUDDY_STRATEGY_MALLOC
#endif


namespace Memory {
	using Data::Bytes;
	using Data::Byte;


	// ------------------------------------------------------------------------
	struct BuddyArena_None {
		static const size_t MAX_BINS = 16;
		static const size_t MIN_SIZE = 64;
	};

	// ------------------------------------------------------------------------
	struct BuddyArena_ByNode {
        static const size_t MAX_BINS = 16;
        static const size_t MIN_SIZE = 64;

		struct Chunk {
			Chunk* prev;
			Chunk* next;
            void* mem;
            void* end;
		};
		struct Block {
            static const size_t HEADER_SIZE = 8;
            static const size_t USED_BIT = 0x10;

			uintptr_t handle;
			Block* prev;
			Block* next;

            bool isfree() const;
            Chunk* owner() const;
            size_t level() const;
            void setattr(size_t lvl, size_t bits);

            static Bytes freespace(Block* block, size_t size);
            static Block* fromptr(void* ptr);
		};

        Block* bins[MAX_BINS];
        Chunk* chunks;

	public:
		 BuddyArena_ByNode();
		~BuddyArena_ByNode();

		Bytes alloc(size_t minsize);
        Bytes realloc(void* ptr, size_t minsize);
		void  dealloc(void* ptr);
	};


#if (M_BUDDY_STRATEGY == M_BUDDY_STRATEGY_BYMASK)
	using BuddyArena = BuddyArena_ByNode;
#elif (M_BUDDY_STRATEGY == M_BUDDY_STRATEGY_BYNODE)
	using BuddyArena = BuddyArena_ByNode;
#else
	using BuddyArena = BuddyArena_ByNode;
#endif


	// ------------------------------------------------------------------------
	class BuddyHeap
		: public IAllocator
	{
	public:
		virtual Bytes alloc(size_t minsize) override;

        Bytes realloc(void* ptr, size_t minsize);
		void dealloc(void* ptr);

	private:
		BuddyArena m_arena;
	};

	// ------------------------------------------------------------------------
    size_t getbuddylevel(size_t size);
    size_t getbuddysize(size_t level);

	extern BuddyHeap buddy_global_heap;


    template <class T>
    void dealloc(T* ptr) { ptr->~T(); buddy_global_heap.dealloc(ptr); }

} // namespace Memory


#endif // MEMORY_BUDDY_HEAP_H