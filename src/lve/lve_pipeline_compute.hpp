#pragma once

#include "lve_device.hpp"

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

    class LveComputePipeline
    {
    public:
        LveComputePipeline(LveDevice &device, const std::string &compFilepath, const ComputePipelineConfigInfo &configInfo);
        ~LveComputePipeline();

        LveComputePipeline(const LveComputePipeline &) = delete;
        LveComputePipeline &operator=(const LveComputePipeline &) = delete;

        VkPipeline getPipeline() { return computePipeline; }

    private:
        void createComputePipeline(const std::string &compFilepath, const ComputePipelineConfigInfo &configInfo);

        LveDevice &lveDevice;
        VkPipeline computePipeline;
        VkShaderModule compShaderModule;
    };
} // namespace lve
