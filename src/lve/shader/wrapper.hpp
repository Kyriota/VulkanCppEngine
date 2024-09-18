#pragma once

#include "lve/core/device.hpp"
#include "lve/core/resource/descriptors.hpp"
#include "lve/shader/parser.hpp"

namespace lve
{
    class ShaderWrapper
    {
    public:
        class Builder
        {
        public:
            Builder(
                Device *device,
                const std::string &spvFilePath,
                VkShaderStageFlagBits shaderType,
                uint32_t maxFramesInFlight
            );
            ShaderWrapper build();

        private:
            Device *device;
            VkShaderStageFlagBits shaderType;
            uint32_t maxFramesInFlight;
        };

    private:
        Device *device;
        std::unique_ptr<lve::DescriptorPool> pool;
    };
} // namespace lve