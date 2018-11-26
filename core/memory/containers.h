#pragma once
#ifndef MEMORY_CONTAINER_H
#define MEMORY_CONTAINER_H

#include "core/memory/plain.h"
#include "core/tools/utils.h"


namespace Memory {
    template <class T>
    class Stack {
    public:
        Stack(size_t startCapacity = 0, BuddyHeap* heap = nullptr);

        void push(T const& elem);
        T&   top() const;
        void pop();

    private:
        Chain m_chain;
    };


    template <class T>
    struct RaStackWrapper {
    public:
        RaStackWrapper();

        RaStackWrapper(RaStackWrapper&& stack);
        RaStackWrapper(const RaStackWrapper& stack) = delete;
        RaStackWrapper& operator=(RaStackWrapper&& stack);
        RaStackWrapper& operator=(const RaStackWrapper& stack) = delete;

        T const& at(Region const& region, size_t idx) const;
        T& at(Region const& region, size_t idx);

        Data::Array<T> asArray(Region const& region) const;
        T* alloc(Region& region);

        template <class... ArgsTy>
        size_t append(Region& region, ArgsTy&&... args);
        void insert(Region& region, Data::Array<T> const& elems);

        size_t count() const { return m_top; }
        size_t last() const { return m_top - 1; }
        T& lastval(Region const& region) const { return *region.get<T>(m_top - 1); }

        void reset() { m_top = 0; }
        void clear(Region& region) { region.clear(); m_top = 0; }

        void pop(size_t cnt) { M_ASSERT(m_top >= cnt); m_top -= cnt; }
        void pop()           { M_ASSERT(m_top != 0); --m_top; }

        bool isEmpty() const { return m_top == 0; }

    private:
        size_t m_top;
    };


    template <class T>
    struct RaStack {
    public:
        RaStack(size_t startCapacity = 0, BuddyHeap* heap = nullptr);

        RaStack(RaStack&& stack);
        RaStack(const RaStack& stack) = delete;
        RaStack& operator=(RaStack&& stack);
        RaStack& operator=(const RaStack& stack) = delete;

        T const& operator[](size_t idx) const { return m_stack.at(m_region, idx); }
        T& operator[](size_t idx) { return m_stack.at(m_region, idx); }

        Data::Array<T> asArray() const { return m_stack.asArray(m_region); }
        T* alloc() { return m_stack.alloc(m_region); }

        template <class... ArgsTy>
        size_t append(ArgsTy&&... args) { return m_stack.append(m_region, std::forward<ArgsTy>(args)...); }

        void insert(Data::Array<T> const& elems) { m_stack.insert(m_region, elems); }

        size_t count() const { return m_stack.count(); }
        size_t last() const { return m_stack.last(); }
        T& lastval() const { return m_stack.lastval(m_region); }

        void reset() { m_stack.reset(); }
        void clear() { m_stack.clear(m_region); }

        void pop(size_t cnt) { m_stack.pop(cnt); }
        void pop()           { m_stack.pop(); }

        bool isEmpty() const { return m_stack.isEmpty(); }

    private:
        RaStackWrapper<T> m_stack;
        Region m_region;
    };


    template <class T>
    struct RaPool {
        struct FreeCell { FreeCell* next; };
    public:
        RaPool(size_t startCapacity = 0, BuddyHeap* heap = nullptr);

        RaPool(const RaPool& arr) = delete;
        RaPool(RaPool&& arr) = delete;

        RaPool& operator=(const RaPool& arr) = delete;
        RaPool& operator=(RaPool&& arr) = delete;

        T& operator[](size_t idx) const { return *m_buffer.get<T>(idx); }

        T* ptr(size_t idx) const { return m_buffer.get<T>(idx); }
        size_t index(T* p) const { return p - (T*)m_buffer.memory.begin; }

        Data::Array<T> asArray() const;

        template <class... ArgsTy>
        size_t emplace(ArgsTy&&... args);

        void dealloc(T* val);
        T* alloc();

        void reset();
        void clear() { m_buffer.clear(); m_free = nullptr; }

        size_t count() const { return m_buffer.size() / sizeof(T); }

    private:
        FreeCell* m_free;
        RegBuffer m_buffer;
    };

    template <class T>
    class Line {
    public:
        M_DECL_MOVE_ONLY(Line);

        Line() = default;

        T const& operator[](size_t idx) const;
        T& operator[](size_t idx);

        Data::Array<T> asArray(size_t n) const;
        Data::Array<T> asArray() const;

    private:
        mutable Region m_region;
    };

} // namespace Memory


#include "hpp/containers.hpp"

#endif // MEMORY_CONTAINER_H