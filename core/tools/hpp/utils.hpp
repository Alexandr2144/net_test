#include "core/tools/utils.h"

#include "core/memory/buddy_heap.h"


namespace Tools {
    // ------------------------------------------------------------------------
    template <class T>
    Array<T> newArray(Memory::IAllocator* a, size_t count)
    {
        if (a == nullptr) a = &Memory::buddy_global_heap;
        return Data::toArray<T>(a->alloc(count * sizeof(T)));
    }

    // ------------------------------------------------------------------------
    template <class T>
    Array<T> newArray(Memory::IAllocator* a, std::initializer_list<T> list)
    {
        if (a == nullptr) a = &Memory::buddy_global_heap;

        size_t count = list.size();
        Array<T> arr = newArray<T>(a, count);

        for (size_t i = 0; i < count; ++i) {
            new(&arr[i]) T(list.begin()[i]);
        }
        return arr;
    }

    // ------------------------------------------------------------------------
    template <class T>
    SparseArray<T> newSparseArray(Memory::IAllocator* a, Data::MemberPointer<T> mptr, size_t count)
    {
        if (a == nullptr) a = &Memory::buddy_global_heap;
        return SparseArray<T>{ a->alloc(count * sizeof(mptr.stride)), mptr.offset, mptr.stride};
    }

    // ------------------------------------------------------------------------
    template <class T, class... ArgsTy>
    Unique<T> newUnique(Memory::IAllocator* a, ArgsTy&&... args)
    {
        if (a == nullptr) a = &Memory::buddy_global_heap;

        Data::Bytes bytes = a->alloc(sizeof(T));
        return new(bytes.begin) T(std::forward<ArgsTy>(args)...);
    }

    // ------------------------------------------------------------------------
    template <class T, class... ArgsTy>
    Array<T> buildArray(Memory::IAllocator* a, size_t count, ArgsTy&&... args)
    {
        Array<T> result = newArray<T>(a, count);
        for (auto& val : iterate(result)) {
            new(&val) T(std::forward<ArgsTy>(args)...);
        }
        return result;
    }
    // ------------------------------------------------------------------------
    template <class T>
    void destroyArray(Array<T> const& array)
    {
        for (auto& val : iterate(array)) {
            val.~T();
        }
        Memory::buddy_global_heap.dealloc(array.begin);
    }

} // namespace Tools