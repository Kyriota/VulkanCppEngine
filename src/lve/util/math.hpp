#pragma once

#ifndef _USE_MATH_DEFINES
    #define _USE_MATH_DEFINES // for math.h
#endif

// libs
#include "include/glm.hpp"

// std
#include <cstddef>
#include <functional>
#include <math.h>

namespace lve::math
{
template <typename T>
T intPow(T base, unsigned int exp);

template <typename T>
T square(T x)
{
    return x * x;
}

template <typename T>
T cube(T x)
{
    return x * x * x;
}

template <typename T, glm::qualifier Q>
T trace(const glm::mat<2, 2, T, Q> &mat)
{
    return mat[0][0] + mat[1][1];
}

template <typename T, glm::qualifier Q>
T trace(const glm::mat<3, 3, T, Q> &mat)
{
    return mat[0][0] + mat[1][1] + mat[2][2];
}

template <typename T, glm::qualifier Q>
T trace(const glm::mat<4, 4, T, Q> &mat)
{
    return mat[0][0] + mat[1][1] + mat[2][2] + mat[3][3];
}

unsigned int positiveMod(int value, unsigned int m);
float fastInvSqrt(float x);
float fastSqrt(float x);
} // namespace lve::math

#include "math.tpp"