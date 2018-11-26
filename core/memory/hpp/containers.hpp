#include "core/memory/containers.h"


namespace Memory {
    // -------------------------------------------------------------------------
    // Stack implementation
    // -------------------------------------------------------------------------
    template <class T>
    Stack<T>::Stack(size_t startCapacity, BuddyHeap* heap)
        : m_chain(startCapacity * sizeof(T), heap)
    {
    }

    // -------------------------------------------------------------------------
    template <class T>
    void Stack<T>::push(T const& elem)
    {
        Data::Bytes bytes = m_chain.reserve(sizeof(T));
        Data::bytecopy(bytes, Data::toBytes(&elem));
    }

    // -------------------------------------------------------------------------
    template <class T>
    T& Stack<T>::top() const
    {
        return *(T*)(m_chain.lastChunk->last - sizeof(T));
    }

    // -------------------------------------------------------------------------
    template <class T>
    void Stack<T>::pop()
    {
        m_chain.back(sizeof(T));
    }

    // -------------------------------------------------------------------------
    // RaStackWrapper implementation
    // -------------------------------------------------------------------------
    template <class T>
    RaStackWrapper<T>::RaStackWrapper()
        : m_top(0)
    {
    }

    // -------------------------------------------------------------------------
    template <class T>
    RaStackWrapper<T>::RaStackWrapper(RaStackWrapper&& stack)
        : m_top(stack.m_top)
    {
        stack.m_top = 0;
    }

    // -------------------------------------------------------------------------
    template <class T>
    RaStackWrapper<T>& RaStackWrapper<T>::operator=(RaStackWrapper&& stack)
    {
        m_top = stack.m_top;
        stack.m_top = 0;
        return *this;
    }

    // -------------------------------------------------------------------------
    template <class T>
    T const& RaStackWrapper<T>::at(Region const& region, size_t idx) const
    {
        M_ASSERT(idx <= m_top);
        return asArray(region).begin[idx];
    }

    // -------------------------------------------------------------------------
    template <class T>
    T& RaStackWrapper<T>::at(Region const& region, size_t idx)
    {
        M_ASSERT(idx <= m_top);
        return asArray(region).begin[idx];
    }

    // -------------------------------------------------------------------------
    template <class T>
    Data::Array<T> RaStackWrapper<T>::asArray(Region const& region) const
    {
        Data::Bytes bytes = region.memory;
        return Data::Array<T>((T*)bytes.begin, (T*)bytes.begin + m_top);
    }

    // -------------------------------------------------------------------------
    template <class T>
    template <class... ArgsTy>
    size_t RaStackWrapper<T>::append(Region& region, ArgsTy&&... args)
    {
        new(alloc(region)) T(std::forward<ArgsTy>(args)...);
        return m_top - 1;
    }

    // -------------------------------------------------------------------------
    template <class T>
    void RaStackWrapper<T>::insert(Region& region, Data::Array<T> const& elems)
    {
        m_top += Data::count(elems);
        region.reserve(m_top * sizeof(T));

        size_t memsize = Data::count(elems) * sizeof(T);
        Data::Byte* mem = region.memory.end - memsize;
        memcpy(mem, elems.begin, memsize);
    }

    // -------------------------------------------------------------------------
    template <class T>
    T* RaStackWrapper<T>::alloc(Region& region)
    {
        region.reserve((++m_top) * sizeof(T));

        Data::Bytes bytes = region.memory;
        return ((T*)bytes.begin) + m_top - 1;
    }

    // -------------------------------------------------------------------------
    // RaStack implementation
    // -------------------------------------------------------------------------
    template <class T>
    RaStack<T>::RaStack(size_t startCapacity, BuddyHeap* heap)
        : m_region(startCapacity * sizeof(T), heap)
    {

    }

    // -------------------------------------------------------------------------
    template <class T>
    RaStack<T>::RaStack(RaStack&& stack)
        : m_region(std::move(stack.m_region))
        , m_stack(std::move(stack.m_stack))
    {
    }

    // -------------------------------------------------------------------------
    template <class T>
    RaStack<T>& RaStack<T>::operator=(RaStack&& stack)
    {
        new(this) RaStack<T>(std::move(stack));
        return *this;
    }

    // -------------------------------------------------------------------------
    // RaPool implementation
    // -------------------------------------------------------------------------
    template <class T>
    RaPool<T>::RaPool(size_t startCapacity, BuddyHeap* heap)
        : m_buffer(startCapacity * sizeof(T), heap)
        , m_free(nullptr)
    {
    }

    // -------------------------------------------------------------------------
    template <class T>
    Data::Array<T> RaPool<T>::asArray() const
    {
        Data::Bytes bytes = m_buffer.memory;
        return Data::Array<T>((T*)bytes.begin, (T*)bytes.begin + m_buffer.size() / sizeof(T));
    }

    // -------------------------------------------------------------------------
    template <class T>
    template <class... ArgsTy>
    size_t RaPool<T>::emplace(ArgsTy&&... args)
    {
        T* p = alloc();
        new(p) T(std::forward<ArgsTy>(args)...);
        return index(p);
    }

    // -------------------------------------------------------------------------
    template <class T>
    void RaPool<T>::dealloc(T* val)
    {
        FreeCell* cell = (FreeCell*)val;
        cell->next = m_free;
        m_free = cell;
    }

    // -------------------------------------------------------------------------
    template <class T>
    T* RaPool<T>::alloc()
    {
        if (m_free) {
            FreeCell* cell = m_free;
            m_free = cell->next;
            return (T*)cell;
        }
        else {
            Bytes bytes = m_buffer.reserve(sizeof(T));
            return (T*)bytes.begin;
        }
    }

    // -------------------------------------------------------------------------
    template <class T>
    void RaPool<T>::reset()
    {
        m_free = nullptr;

        Byte* ptr = m_buffer.memory.begin;
        while (ptr < m_buffer.memory.end) {
            dealloc((T*)ptr);
            ptr += sizeof(T);
        }
    }

    // -------------------------------------------------------------------------
    // Line implementation
    // -------------------------------------------------------------------------
    template <class T>
    Line<T>::Line(Line&& line)
        : m_region(std::move(line.m_region))
    {
    }

    // -------------------------------------------------------------------------
    template <class T>
    Line<T>& Line<T>::operator=(Line&& arr)
    {
        new(this) Line(std::move(arr));
        return *this;
    }

    // -------------------------------------------------------------------------
    template <class T>
    T const& Line<T>::operator[](size_t idx) const
    {
        return asArray(idx).begin[idx];
    }

    // -------------------------------------------------------------------------
    template <class T>
    T& Line<T>::operator[](size_t idx)
    {
        return asArray(idx).begin[idx];
    }

    // -------------------------------------------------------------------------
    template <class T>
    Data::Array<T> Line<T>::asArray(size_t n) const
    {
        m_region.reserve(sizeof(T) * (n + 1));

        Data::Bytes bytes = m_region.memory;
        return Data::Array<T>{ (T*)bytes.begin, (T*)bytes.begin + n };
    }

} // namespace Memory