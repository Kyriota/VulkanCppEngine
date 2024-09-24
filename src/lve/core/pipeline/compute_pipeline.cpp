#include "lve/core/pipeline/compute_pipeline.hpp"

#include "lve/GO/geo/model.hpp"
#include "lve/path.hpp"
#include "lve/util/config.hpp"
#include "lve/util/file_io.hpp"

// std
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace lve
{
ComputePipeline::ComputePipeline(
    Device &device,
    const std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
    const std::string &compFilePath
)
    : Pipeline(device)
{
    createPipelineLayout(descriptorSetLayouts);
    createPipeline(compFilePath);
    initialized = true;
}

void ComputePipeline::dispatchComputePipeline(
    VkCommandBuffer cmdBuffer,
    const VkDescriptorSet *pGlobalDescriptorSet,
    uint32_t width,
    uint32_t height
)
{
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(
        cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, pGlobalDescriptorSet, 0, nullptr
    );

    vkCmdDispatch(cmdBuffer, width, height, 1);
}

void ComputePipeline::bind(VkCommandBuffer commandBuffer)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
}

void ComputePipeline::createPipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(lveDevice.vkDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout");
    }
}

void ComputePipeline::createPipeline(const std::string &compFilePath)
{
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout is initialized");

    std::vector<char> compCode;
    io::readBinaryFile(lve::path::asset::SHADER + compFilePath, compCode);
    initShaderModule("comp", compCode);

    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageInfo.module = shaderModules["comp"];
    shaderStageInfo.pName = "main";

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateComputePipelines(
            lveDevice.vkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline
        ) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create compute pipeline");
    }
}
} // namespace lve
