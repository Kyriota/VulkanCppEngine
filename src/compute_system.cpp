#include "compute_system.hpp"

// std
#include <cassert>

namespace lve
{
    ComputeSystem::ComputeSystem(LveDevice &device)
        : lveDevice{device}
    {
        createComputePipelineLayout();
        createComputePipeline();
    }

    ComputeSystem::~ComputeSystem()
    {
        vkDestroyPipelineLayout(lveDevice.device(), computePipelineLayout, nullptr);
    }

    void ComputeSystem::dispatchComputePipeline(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, lveComputePipeline->getPipeline());
        vkCmdDispatch(commandBuffer, width, height, 1);
    }

    void ComputeSystem::createComputePipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &computePipelineLayout) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void ComputeSystem::createComputePipeline()
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