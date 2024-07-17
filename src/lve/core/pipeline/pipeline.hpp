#pragma once

#include "lve/core/device.hpp"

namespace lve
{
    class Pipeline
    {
    public:
        Pipeline(Device &device) : lveDevice(device) {}
        ~Pipeline() { cleanUp(); }

        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;

        Pipeline(Pipeline &&other) noexcept;
        Pipeline &operator=(Pipeline &&other);

        VkPipeline getPipeline() { return pipeline; }
        VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

    protected:
        void cleanUp();

        Device &lveDevice;
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
        std::unordered_map<std::string, VkShaderModule> shaderModules;

        bool initialized = false;
    };
} // namespace lve