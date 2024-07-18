#include "include/spirv_cross.hpp"

namespace lve
{
    class SpirvParser
    {
    public:
        SpirvParser(const std::vector<uint32_t> &spirv_binary);

    private:
        spirv_cross::ShaderResources resources;
    };
} // namespace lve