#pragma once

#include <cstddef>
#include <vector>

namespace lve
{
// from: https://stackoverflow.com/a/57595105
template <typename T, typename... Rest>
void hashCombine(std::size_t &seed, const T &v, const Rest &...rest)
{
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hashCombine(seed, rest), ...);
};

// hash a vector
template <typename T>
std::size_t hash(const std::vector<T> &v)
{
    std::size_t seed = 0;
    for (const T &i : v)
    {
        hashCombine(seed, i);
    }
    return seed;
}
} // namespace lve