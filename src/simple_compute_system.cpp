#include "simple_compute_system.hpp"

// std
#include <cassert>

namespace lve
{
    SimpleComputeSystem::SimpleComputeSystem(LveDevice &device)
        : lveDevice{device}
    {
        createComputePipelineLayout();
        createComputePipeline();
    }

    SimpleComputeSystem::~SimpleComputeSystem()
    {
        vkDestroyPipelineLayout(lveDevice.device(), computePipelineLayout, nullptr);
    }

    void SimpleComputeSystem::dispatchComputePipeline(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, lveComputePipeline->getPipeline());
        vkCmdDispatch(commandBuffer, width, height, 1);
    }

    void SimpleComputeSystem::createComputePipelineLayout()
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = 0; // Optional

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &computePipelineLayout) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void SimpleComputeSystem::createComputePipeline()
    {
        assert(computePipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        ComputePipelineConfigInfo computePipelineConfig{};
        computePipelineConfig.pipelineLayout = computePipelineLayout;
        lveComputePipeline = std::make_unique<LveComputePipeline>(
            lveDevice,
            "shaders/my_compute_shader.comp.spv",
            computePipelineConfig);
    }
} // namespace lve