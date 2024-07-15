#include "lve/core/system/compute_system.hpp"

// std
#include <cassert>

namespace lve
{
    ComputeSystem::ComputeSystem(
        Device &device,
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
        const std::string &compFilepath)
        : lveDevice{device}
    {
        createComputePipelineLayout(descriptorSetLayouts);
        createComputePipeline(compFilepath);
    }

    ComputeSystem::~ComputeSystem()
    {
        vkDestroyPipelineLayout(lveDevice.vkDevice(), computePipelineLayout, nullptr);
    }

    ComputeSystem::ComputeSystem(ComputeSystem &&other) noexcept
        : lveDevice{other.lveDevice},
          lveComputePipeline{std::move(other.lveComputePipeline)},
          computePipelineLayout{other.computePipelineLayout},
          initialized{other.initialized}
    {
        other.computePipelineLayout = nullptr;
    }

    ComputeSystem &ComputeSystem::operator=(ComputeSystem &&other)
    {
        if (this->lveDevice.vkDevice() != other.lveDevice.vkDevice())
        {
            throw std::runtime_error("Moved ComputeSystem objects must be on the same LveDevice");
        }

        if (this != &other)
        {
            // Clean up existing resources
            cleanUp();

            lveComputePipeline = std::move(other.lveComputePipeline);
            computePipelineLayout = other.computePipelineLayout;
            initialized = other.initialized;

            // Reset other object
            other.computePipelineLayout = nullptr;
        }

        return *this;
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

    void ComputeSystem::cleanUp()
    {
        if (initialized)
        {
            vkDestroyPipelineLayout(lveDevice.vkDevice(), computePipelineLayout, nullptr);
        }
    }

    void ComputeSystem::createComputePipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts)
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        
        if (vkCreatePipelineLayout(lveDevice.vkDevice(), &pipelineLayoutInfo, nullptr, &computePipelineLayout) !=
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
        lveComputePipeline = std::make_unique<ComputePipeline>(
            lveDevice,
            compFilepath,
            computePipelineConfig);
    }
} // namespace lve