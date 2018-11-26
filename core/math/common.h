#pragma once
#ifndef DATA_MATH_H
#define DATA_MATH_H


namespace Math {
    int log2(int x);
    int bitscount(int x);

    size_t align(size_t x, size_t alignment);

    template <class T>  T min(T const& a, T const& b) { return a < b ? a : b; }
    template <class T>  T max(T const& a, T const& b) { return a > b ? a : b; }

    template <class T>  T clamp(T a, T b, T t);

} // namespace Data


#include "hpp/common.hpp"

#endif // DATA_MATH_H