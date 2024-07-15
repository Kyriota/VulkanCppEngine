#include "lve/core/pipeline/compute_pipeline.hpp"
#include "lve/core/pipeline/pipeline_op.hpp"
#include "lve/go/geo/model.hpp"
#include "lve/util/file_io.hpp"

// std
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace lve
{
    ComputePipeline::ComputePipeline(Device &device,
                                     const std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
                                     const std::string &compFilepath)
        : lveDevice{device}
    {
        createComputePipelineLayout(descriptorSetLayouts);
        createComputePipeline(compFilepath);
        initialized = true;
    }

    ComputePipeline::~ComputePipeline() { cleanUp(); }

    ComputePipeline::ComputePipeline(ComputePipeline &&other) noexcept : lveDevice{other.lveDevice}
    {
        computePipeline = other.computePipeline;
        computePipelineLayout = other.computePipelineLayout;
        compShaderModule = other.compShaderModule;
        initialized = other.initialized;

        // Reset other object
        other.computePipeline = VK_NULL_HANDLE;
        other.computePipelineLayout = VK_NULL_HANDLE;
        other.compShaderModule = VK_NULL_HANDLE;
        other.initialized = false;
    }

    ComputePipeline &ComputePipeline::operator=(ComputePipeline &&other)
    {
        if (this->lveDevice.vkDevice() != other.lveDevice.vkDevice())
        {
            throw std::runtime_error("Moved ComputePipeline objects must be on the same VkDevice");
        }

        if (this != &other)
        {
            // Clean up existing resources
            cleanUp();

            computePipeline = other.computePipeline;
            computePipelineLayout = other.computePipelineLayout;
            compShaderModule = other.compShaderModule;
            initialized = other.initialized;

            // Reset other object
            other.computePipeline = VK_NULL_HANDLE;
            other.computePipelineLayout = VK_NULL_HANDLE;
            other.compShaderModule = VK_NULL_HANDLE;
            other.initialized = false;
        }

        return *this;
    }

    void ComputePipeline::dispatchComputePipeline(VkCommandBuffer cmdBuffer,
                                                  const VkDescriptorSet *pGlobalDescriptorSet,
                                                  uint32_t width, uint32_t height)
    {
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0,
                                1, pGlobalDescriptorSet, 0, nullptr);

        vkCmdDispatch(cmdBuffer, width, height, 1);
    }

    void ComputePipeline::cleanUp()
    {
        if (initialized)
        {
            vkDestroyPipelineLayout(lveDevice.vkDevice(), computePipelineLayout, nullptr);
            vkDestroyShaderModule(lveDevice.vkDevice(), compShaderModule, nullptr);
            vkDestroyPipeline(lveDevice.vkDevice(), computePipeline, nullptr);
            initialized = false;
        }
    }

    void ComputePipeline::createComputePipelineLayout(
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts)
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(lveDevice.vkDevice(), &pipelineLayoutInfo, nullptr,
                                   &computePipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout");
        }
    }

    void ComputePipeline::createComputePipeline(const std::string &compFilepath)
    {
        assert(computePipelineLayout != nullptr &&
               "Cannot create pipeline before pipeline layout is created");

        ComputePipelineConfigInfo configInfo{};
        configInfo.pipelineLayout = computePipelineLayout;

        assert(configInfo.pipelineLayout != VK_NULL_HANDLE &&
               "Cannot create compute pipeline: no pipelineLayout provided in configInfo");

        io::YamlConfig generalConfig{"config/general.yaml"};
        std::string shaderRoot = generalConfig.get<std::string>("shaderRoot") + "/";
        std::vector<char> compCode = io::readBinaryFile(shaderRoot + compFilepath);
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

        if (vkCreateComputePipelines(lveDevice.vkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo,
                                     nullptr, &computePipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create compute pipeline");
        }
    }
} // namespace lve
