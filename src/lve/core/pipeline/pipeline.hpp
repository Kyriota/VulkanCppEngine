#pragma once

#include "lve/core/device.hpp"

// std
#include <string>

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

        virtual void bind(VkCommandBuffer commandBuffer) = 0;

    protected:
        void cleanUp();
        void initShaderModule(std::string moduleName, const std::vector<char> &code);

        Device &lveDevice;
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
        std::unordered_map<std::string, VkShaderModule> shaderModules;

        bool initialized = false;
    };
} // namespace lve