#pragma once
#ifndef TOOLS_UTILS_H
#define TOOLS_UTILS_H

#include "native/crash.h"
#include "core/data/array.h"
#include "core/data/pointers.h"
#include "core/memory/common.h"


#define M_TAGGED_TYPE(NAME, TYPE, INVALID)      \
	struct NAME {                               \
		TYPE value = (TYPE)INVALID;             \
		bool isValid() const { return value != (TYPE)INVALID; }            \
		bool operator==(NAME const& b) const { return value == b.value; }  \
		bool operator!=(NAME const& b) const { return value == b.value; }  \
	}

#define M_DECL_MOVE_ONLY(TYPE)          \
    TYPE(TYPE&&);                \
    TYPE(TYPE const&) = delete;       \
    TYPE& operator=(TYPE&&);          \
    TYPE& operator=(TYPE const&) = delete


template <class ToTy, class FromTy>
ToTy narrow_cast(Native::CL cl, FromTy const& val) { assert(M_COND_E(cl, val == (ToTy)val)); return (ToTy)val; }

namespace Tools {
    using Data::SparseArray;
    using Data::Array;
    using Data::Unique;

    template <class T>   Array<T> newArray(Memory::IAllocator* a, size_t count);
    template <class T>   Array<T> newArray(Memory::IAllocator* a, std::initializer_list<T> list);

    template <class T>   SparseArray<T> newSparseArray(Memory::IAllocator* a, Data::MemberPointer<T> mptr, size_t count);

    template <class T, class... ArgsTy>
    Unique<T> newUnique(Memory::IAllocator* a, ArgsTy&&... args);


    template <class T, class... ArgsTy>
    Array<T> buildArray(Memory::IAllocator* a, size_t count, ArgsTy&&... args);

    template <class T>
    void destroyArray(Array<T> const& array);

} // namespace Tools


#include "hpp/utils.hpp"

#endif // TOOLS_UTILS_H