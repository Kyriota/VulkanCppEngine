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
        ComputePipeline(Device &device) : lveDevice(device) {}
        ComputePipeline(Device &device,
                        const std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
                        const std::string &compFilepath);
        ~ComputePipeline();

        ComputePipeline(const ComputePipeline &) = delete;
        ComputePipeline &operator=(const ComputePipeline &) = delete;

        ComputePipeline(ComputePipeline &&other) noexcept;
        ComputePipeline &operator=(ComputePipeline &&other);

        VkPipeline getPipeline() { return computePipeline; }
        VkPipelineLayout getPipelineLayout() { return computePipelineLayout; }

        void dispatchComputePipeline(VkCommandBuffer cmdBuffer,
                                     const VkDescriptorSet *pGlobalDescriptorSet, uint32_t width,
                                     uint32_t height);

    private:
        void cleanUp();

        void createComputePipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
        void createComputePipeline(const std::string &compFilepath);

        Device &lveDevice;
        VkPipeline computePipeline;
        VkPipelineLayout computePipelineLayout;
        VkShaderModule compShaderModule;

        bool initialized = false;
    };
} // namespace lve
