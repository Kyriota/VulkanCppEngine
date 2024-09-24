#pragma once

#include "lve/core/device.hpp"
#include "lve/core/pipeline/pipeline_base.hpp"

// std
#include <string>
#include <vector>

namespace lve
{
class ComputePipeline : public Pipeline
{
public:
    ComputePipeline(Device &device) : Pipeline(device) {}
    ComputePipeline(
        Device &device,
        const std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
        const std::string &compFilePath
    );

    void dispatchComputePipeline(
        VkCommandBuffer cmdBuffer,
        const VkDescriptorSet *pGlobalDescriptorSet,
        uint32_t width,
        uint32_t height
    );

    void bind(VkCommandBuffer commandBuffer) override;

private:
    void createPipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
    void createPipeline(const std::string &compFilePath);
};
} // namespace lve
