namespace lve
{
    namespace LveMath
    {
        template <typename T>
        T intPow(T base, unsigned int exp)
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
    } // namespace LveMath
} // namespace lve