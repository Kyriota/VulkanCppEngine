#pragma once

namespace lve
{
    namespace LveMath
    {
        template <typename T>
        T intPow(T base, unsigned int exp);

        unsigned int positiveMod(int value, unsigned int m);
    } // namespace math
} // namespace lve

#include "lve_math.tpp"