#include "lve/util/math.hpp"

namespace lve
{
    namespace math
    {
        unsigned int positiveMod(int value, unsigned int m)
        {
            int mod = value % (int)m;
            if (mod < 0)
                mod += m;
            return mod;
        }
    } // namespace math
} // namespace lve