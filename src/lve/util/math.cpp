#include "lve/util/math.hpp"

namespace lve::math
{
unsigned int positiveMod(int value, unsigned int m)
{
    int mod = value % (int)m;
    if (mod < 0)
        mod += m;
    return mod;
}

float fastInvSqrt(float x)
{
    long i;
    float halfNum = x * 0.5f;

    i = *(long *)&x;
    i = 0x5f3759df - (i >> 1);
    x = *(float *)&i;
    x *= 1.5f - (halfNum * x * x);
    x *= 1.5f - (halfNum * x * x);

    return x;
}

float fastSqrt(float x) { return x * fastInvSqrt(x); }
} // namespace lve::math