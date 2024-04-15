#include "compute_system.hpp"

// std
#include <cassert>

namespace lve
{
    ComputeSystem::ComputeSystem(LveDevice &device, const std::string &compFilepath)
        : lveDevice{device}
    {
        createComputePipelineLayout();
        createComputePipeline(compFilepath);
    }

    ComputeSystem::~ComputeSystem()
    {
        vkDestroyPipelineLayout(lveDevice.device(), computePipelineLayout, nullptr);
    }

    void ComputeSystem::dispatchComputePipeline(FrameInfo frameInfo, uint32_t width, uint32_t height)
    {
        vkCmdBindPipeline(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, lveComputePipeline->getPipeline());
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            computePipelineLayout,
            0,
            1,
            &frameInfo.textureStorageDescriptorSet,
            0,
            nullptr);
        vkCmdDispatch(frameInfo.commandBuffer, width, height, 1);
    }

    void ComputeSystem::createComputePipelineLayout()
    {
        VkDescriptorSetLayoutBinding storageImageLayoutBinding{};
        storageImageLayoutBinding.binding = 0;
        storageImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        storageImageLayoutBinding.descriptorCount = 1;
        storageImageLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.bindingCount = 1;
        descriptorSetLayoutCreateInfo.pBindings = &storageImageLayoutBinding;

        VkDescriptorSetLayout descriptorSetLayout;
        if (vkCreateDescriptorSetLayout(lveDevice.device(), &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
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