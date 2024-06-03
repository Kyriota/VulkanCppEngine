#pragma once

#include "lve/core/device.hpp"

// std
#include <string>
#include <vector>

namespace lve
{
    struct ComputePipelineConfigInfo
    {
        ComputePipelineConfigInfo() = default;
        ComputePipelineConfigInfo(const ComputePipelineConfigInfo &) = delete;
        ComputePipelineConfigInfo &operator=(const ComputePipelineConfigInfo &) = delete;

        VkPipelineLayout pipelineLayout = nullptr;
    };

    class ComputePipeline
    {
    public:
        ComputePipeline(Device &device, const std::string &compFilepath, const ComputePipelineConfigInfo &configInfo);
        ~ComputePipeline();

        ComputePipeline(const ComputePipeline &) = delete;
        ComputePipeline &operator=(const ComputePipeline &) = delete;

        VkPipeline getPipeline() { return computePipeline; }

    private:
        void createComputePipeline(const std::string &compFilepath, const ComputePipelineConfigInfo &configInfo);

        Device &lveDevice;
        VkPipeline computePipeline;
        VkShaderModule compShaderModule;
    };
} // namespace lve
