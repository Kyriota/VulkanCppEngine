#pragma once

namespace lve
{
    namespace math
    {
        template <typename T> T intPow(T base, unsigned int exp)
        {
            T result = 1;
            while (exp)
            {
                if (exp & 1)
                    result *= base;
                exp >>= 1;
                base *= base;
            }
            return result;
        }

        // from: https://stackoverflow.com/a/57595105
        template <typename T, typename... Rest> void hashCombine(std::size_t &seed, const T &v, const Rest &...rest)
        {
            seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            (hashCombine(seed, rest), ...);
        };
    } // namespace math
} // namespace lve