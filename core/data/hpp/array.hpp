#include "core/data/array.h"

#include <memory.h>


namespace Data {
    // ------------------------------------------------------------------------
    template <class T>
    Array<T>::Array(Array<T>&& a)
        : begin(a.begin), end(a.end)
    {
        a = Array<T>();
    }

    // ------------------------------------------------------------------------
    template <class T>
    Array<T>& Array<T>::operator=(Array<T>&& a)
    { 
        memcpy(this, &a, sizeof(Array<T>));
        memset(&a, 0, sizeof(Array<T>));
        return *this;
    }

    // ------------------------------------------------------------------------
    template <class T>
    bool can_split(Array_CRef<T> base, size_t splitSize)
    {
        return base.begin + splitSize <= base.end;
    }

    // ------------------------------------------------------------------------
    template <class T>
    bool can_slice(Array_CRef<T> base, size_t from, size_t to)
    {
        return (base.begin + to <= base.end) && (base.begin + from <= base.end);
    }

    // ------------------------------------------------------------------------
    template <class T>
    Array<T> split(Ref<Array<T>> base, size_t splitSize)
    {
        M_ASSERT(can_split(base.ref, splitSize));

        Array<T> part{ base.ref.begin, base.ref.begin + splitSize };
        base.ref.begin += splitSize;
        return part;
    }

    // ------------------------------------------------------------------------
    template <class T>
    Array<T> slice(Array_CRef<T> base, size_t from, size_t to)
    {
        M_ASSERT(can_slice(base, from, to));
        return Array<T>{ base.begin + from, base.begin + to };
    }

    // ------------------------------------------------------------------------
    template <class T>
    bool read(Ref<CBytes> src, T* dest)
    {
        if (!can_split(src.ref, sizeof(T))) return false;

        *dest = *(T*)split<CByte>(&src, sizeof(T)).begin;
        return true;
    }

    // ------------------------------------------------------------------------
    template <class T>
    void replace(Array_CRef<T> a, T const& from, T const& to)
    {
        for (T& val : iterate(a)) {
            if (val == from) val = to;
        }
    }

    // ------------------------------------------------------------------------
    template <class T>
    bool contain(Array_CRef<T> a, T const& val)
    {
        for (T const& x : iterate(a)) {
            if (x == val) return true;
        }
        return false;
    }

    // ------------------------------------------------------------------------
    template <class T>
    ArrayCollection<T>::ArrayCollection(Array_CRef<T> a)
        : m_array(a)
    {
    }

    // ------------------------------------------------------------------------
    template <class T>
    SparseArrayCollection<T>::SparseArrayCollection(SparseArray_CRef<T> a)
        : m_array(a)
    {
    }

    // ------------------------------------------------------------------------
    template <class T>
    typename SparseArrayCollection<T>::Iterator
        SparseArrayCollection<T>::begin() const
    {
        uint8_t* ptr = (uint8_t*)m_array.memory.begin + m_array.offset;
        return Iterator(ptr, m_array.stride);
    }

    // ------------------------------------------------------------------------
    template <class T>
    typename SparseArrayCollection<T>::Iterator
        SparseArrayCollection<T>::end() const
    {
        uint8_t* ptr = (uint8_t*)m_array.memory.end + m_array.offset;
        return Iterator(ptr, m_array.stride);
    }

    // ------------------------------------------------------------------------
    template <class T>
    SparseArrayCollection<T>::Iterator::Iterator(uint8_t* base, size_t stride)
        : stride(stride)
        , base((Byte*)base)
    {
    }

    // ------------------------------------------------------------------------
    template <class T>
    typename SparseArrayCollection<T>::Iterator&
        SparseArrayCollection<T>::Iterator::operator++(int)
    {
        Iterator save = *this;
        this->operator++();
        return save;
    }

    // ------------------------------------------------------------------------
    template <class T>
    typename SparseArrayCollection<T>::Iterator&
        SparseArrayCollection<T>::Iterator::operator++()
    {
        base = (Byte*)((uint8_t*)base + stride);
        return *this;
    }

} // namespace Data