#pragma once
#ifndef MEMORY_COMMON_H
#define MEMORY_COMMON_H

#include "core/data/array.h"

#undef realloc


namespace Memory {
    using Data::Bytes;

    struct IAllocator {
        virtual Bytes alloc(size_t size) = 0;
    };

    struct IStackAllocator : public IAllocator {
        virtual void dealloc(size_t size) = 0;
        virtual bool empty() const = 0;
    };

    struct IAutoAllocator : public IAllocator {
        virtual bool empty() const = 0;
        virtual void* dealloc() = 0;
    };


    template <class T, class... ArgsTy>
    static T* emplace(IAllocator& allocator, ArgsTy&&... args);

} // namespace Memory


#endif // MEMORY_COMMON_H