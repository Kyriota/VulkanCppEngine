#include "lve_pipeline_compute.hpp"
#include "lve_pipeline_op.hpp"
#include "lve_model.hpp"
#include "lve_file_io.hpp"

// std
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace lve
{
    LveComputePipeline::LveComputePipeline(LveDevice &device, const std::string &compFilepath, const ComputePipelineConfigInfo &configInfo)
        : lveDevice{device}
    {
        createComputePipeline(compFilepath, configInfo);
    }

    LveComputePipeline::~LveComputePipeline()
    {
        vkDestroyShaderModule(lveDevice.device(), compShaderModule, nullptr);
        vkDestroyPipeline(lveDevice.device(), computePipeline, nullptr);
    }

    void LveComputePipeline::createComputePipeline(const std::string &compFilepath, const ComputePipelineConfigInfo &configInfo)
    {
        assert(
            configInfo.pipelineLayout != VK_NULL_HANDLE &&
            "Cannot create compute pipeline: no pipelineLayout provided in configInfo");

        std::vector<char> compCode = readBinaryFile(compFilepath);
        createShaderModule(lveDevice, compCode, &compShaderModule);

        VkPipelineShaderStageCreateInfo shaderStageInfo{};
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStageInfo.module = compShaderModule;
        shaderStageInfo.pName = "main";

        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.stage = shaderStageInfo;
        pipelineInfo.layout = configInfo.pipelineLayout;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        if (vkCreateComputePipelines(
                lveDevice.device(),
                VK_NULL_HANDLE,
                1,
                &pipelineInfo,
                nullptr,
                &computePipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create compute pipeline");
        }
    }
} // namespace lve
