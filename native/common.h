#pragma once
#ifndef NATIVE_COMMON_H
#define NATIVE_COMMON_H

#include <intsafe.h>
#include <stdint.h>
#include <stddef.h>


#undef assert

#ifdef _WIN32
#  define M_EXPORT __declspec(dllexport)
#else
#  define M_EXPORT
#endif


static constexpr size_t SIZE_T_LAST_BIT = (size_t(1)) << (size_t(8 * sizeof(size_t) - 1));

inline void* operator new(size_t size, void* ptr);


#endif // NATIVE_COMMON_H