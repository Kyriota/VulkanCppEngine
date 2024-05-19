#include "compute_system.hpp"

// std
#include <cassert>

namespace lve
{
    ComputeSystem::ComputeSystem(
        LveDevice &device,
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
        const std::string &compFilepath)
        : lveDevice{device}
    {
        createComputePipelineLayout(descriptorSetLayouts);
        createComputePipeline(compFilepath);
    }

    ComputeSystem::~ComputeSystem()
    {
        vkDestroyPipelineLayout(lveDevice.device(), computePipelineLayout, nullptr);
    }

    void ComputeSystem::dispatchComputePipeline(
        VkCommandBuffer cmdBuffer,
        const VkDescriptorSet *pGlobalDescriptorSet,
        uint32_t width, uint32_t height)
    {
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, lveComputePipeline->getPipeline());
        vkCmdBindDescriptorSets(
            cmdBuffer,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            computePipelineLayout,
            0,
            1,
            pGlobalDescriptorSet,
            0,
            nullptr);
        vkCmdDispatch(cmdBuffer, width, height, 1);
    }

    void ComputeSystem::createComputePipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts)
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        
        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &computePipelineLayout) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void ComputeSystem::createComputePipeline(const std::string &compFilepath)
    {
        assert(computePipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        ComputePipelineConfigInfo computePipelineConfig{};
        computePipelineConfig.pipelineLayout = computePipelineLayout;
        lveComputePipeline = std::make_unique<LveComputePipeline>(
            lveDevice,
            compFilepath,
            computePipelineConfig);
    }
} // namespace lve