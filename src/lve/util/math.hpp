#pragma once

#include <cstddef>
#include <functional>

namespace lve
{
    namespace math
    {
        template <typename T>
        T intPow(T base, unsigned int exp);

        template <typename T, typename... Rest>
        void hashCombine(std::size_t &seed, const T &v, const Rest &...rest);

        unsigned int positiveMod(int value, unsigned int m);
    } // namespace math
} // namespace lve

#include "lve/util/math.tpp"