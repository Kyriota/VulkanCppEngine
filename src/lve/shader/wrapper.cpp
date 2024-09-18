#include "lve/shader/wrapper.hpp"

// std
#include <unordered_map>

namespace lve
{
    ShaderWrapper::Builder::Builder(
        Device *device,
        const std::string &spvFilePath,
        VkShaderStageFlagBits shaderType,
        uint32_t maxFramesInFlight
    )
        : device(device), shaderType(shaderType), maxFramesInFlight(maxFramesInFlight)
    {
    }

    ShaderWrapper ShaderWrapper::Builder::build()
    {
        // ShaderParser parser(spvFilePath);

        // DescriptorPool::Builder poolBuilder(*device);
        // std::unordered_map<spv::StorageClass, uint32_t> typeCounts;
        // for (const auto &var : parser.getSummary().variables);
    }
} // namespace lve