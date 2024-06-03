#include "lve_math.hpp"

namespace lve
{
    namespace LveMath
    {
        unsigned int positiveMod(int value, unsigned int m)
        {
            int mod = value % (int)m;
            if (mod < 0)
                mod += m;
            return mod;
        }
    } // namespace LveMath
} // namespace lve