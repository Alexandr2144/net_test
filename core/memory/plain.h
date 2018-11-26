#pragma once
#ifndef MEMORY_PLAIN_H
#define MEMORY_PLAIN_H

#include "core/memory/buddy_heap.h"
#include "core/memory/common.h"

#include "core/data/pointers.h"
#include "core/data/array.h"


namespace Memory {
    using Data::Bytes;
    using Data::Byte;
    using Data::Ref;


    class Chain {
    public:
        struct Chunk {
            Chunk* prev;
            Chunk* next;
            Byte* last;
            Byte* end;

        public:
            size_t size()  const { return last - (Byte*)this; }
            void   clear() { last = (Byte*)this; }
        };

    public:
        Chunk* lastChunk;

        Chain(size_t startsize = 0, BuddyHeap* heap = nullptr);
        ~Chain();

        Chain(Chain&& chain);
        Chain(Chain const& chain) = delete;
        Chain& operator=(Chain&& chain);
        Chain& operator=(Chain const& chain) = delete;

        Bytes reserve(size_t count);
        Bytes forward(size_t count);
        void clear(size_t startsize = 0);
        void back(size_t count);

    private:
        BuddyHeap& m_heap;
        size_t m_nextsize;

        bool has_space(size_t count);
        void extend(size_t count, bool init);
        void dealloc();
    };


    class ChainAllocator
        : public Memory::IAllocator
    {
    public:
        ChainAllocator(size_t startsize = 0, BuddyHeap* heap = nullptr)
            : m_chain(startsize, heap) {}

        virtual Bytes alloc(size_t size) override { return m_chain.reserve(size); }

    private:
        Chain m_chain;
    };


    class ChainBuffer
        : public Chain
    {
    public:
        ChainBuffer(size_t startsize = 0, BuddyHeap* heap = nullptr);

        ChainBuffer(ChainBuffer&& buf);
        ChainBuffer(ChainBuffer const& buf) = delete;
        ChainBuffer& operator=(ChainBuffer&& buf);
        ChainBuffer& operator=(ChainBuffer const& buf) = delete;

        void clear(size_t startsize = 0);

        Bytes reserve(size_t count) { m_totalSize += count; return Chain::reserve(count); }
        Bytes forward(size_t count) { m_totalSize += count; return Chain::forward(count); }
        void back(size_t count) { m_totalSize -= count; Chain::back(count); }

        size_t size() const { return m_totalSize; }

        void extract(Bytes dest) const;
        void extract(Byte* dest) const;

    private:
        Chain::Chunk* m_firstChunk;
        size_t m_totalSize;
    };


    class Region {
    public:
        Bytes memory;

        Region(size_t startsize = 0, BuddyHeap* heap = nullptr);
        ~Region();

        Region(Region&& reg);
        Region(Region const& reg) = delete;
        Region& operator=(Region&& reg);
        Region& operator=(Region const& reg) = delete;

        template <class T>
        T* get(size_t idx) const { return (T*)get(idx, sizeof(T)); }
        void* get(size_t idx, size_t size) const;

        Bytes reserve(size_t offset);
        void clear();

    private:
        BuddyHeap& m_heap;
    };


    class RegBuffer
        : public Region
    {
    public:
        RegBuffer(size_t startsize = 0, BuddyHeap* heap = nullptr);

        RegBuffer(RegBuffer&& chain);
        RegBuffer(RegBuffer const& chain) = delete;
        RegBuffer& operator=(RegBuffer&& chain);
        RegBuffer& operator=(RegBuffer const& chain) = delete;

        void clear() { Region::clear(); m_totalSize = 0; }
        void reset() { m_totalSize = 0; }

        Bytes reserve(size_t count);
        void back(size_t count) { m_totalSize -= count; }

        size_t size() const { return m_totalSize; }

        void extract(Bytes const& dest) const;
        void extract(Byte* dest) const;

    private:
        size_t m_totalSize;
    };


    class Pool {
        struct FreeCell { FreeCell* next; };
    public:
        Pool(size_t startsize = 0, BuddyHeap* heap = nullptr);

        Pool(Pool&& pool);
        Pool(Pool const& pool) = delete;
        Pool& operator=(Pool&& pool);
        Pool& operator=(Pool const& pool) = delete;

        void clear(size_t startsize = 0);
        void reset(size_t cellSize);

        void* alloc(size_t size);
        void dealloc(void* ptr);

        template <class T>
        T* alloc() { return (T*)alloc(sizeof(T)); }

    private:
        FreeCell* m_first;
        Chain m_chain;

        void reset_chunk(Chain::Chunk* chunk, size_t cellSize);
    };

} // namespace Memory


#endif // MEMORY_PLAIN_H