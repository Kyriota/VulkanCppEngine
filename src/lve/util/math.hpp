#pragma once

// std
#include <cstddef>
#include <functional>

namespace lve
{
    namespace math
    {
        template <typename T> T intPow(T base, unsigned int exp);

        unsigned int positiveMod(int value, unsigned int m);
        float fastInvSqrt(float x);
        float fastSqrt(float x);
    } // namespace math
} // namespace lve

#include "lve/util/math.tpp"