#include "lve/core/pipeline/graphics_pipeline.hpp"
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

    GraphicPipelineConfigInfo::GraphicPipelineConfigInfo()
    {
        this->inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        this->inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        this->inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
        this->inputAssemblyInfo.flags = 0;
        this->inputAssemblyInfo.pNext = nullptr;

        this->viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        this->viewportInfo.viewportCount = 1;
        this->viewportInfo.pViewports = nullptr;
        this->viewportInfo.scissorCount = 1;
        this->viewportInfo.pScissors = nullptr;
        this->viewportInfo.flags = 0;
        this->viewportInfo.pNext = nullptr;

        this->rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        this->rasterizationInfo.depthClampEnable = VK_FALSE;
        this->rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        this->rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        this->rasterizationInfo.lineWidth = 1.0f;
        this->rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        this->rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        this->rasterizationInfo.depthBiasEnable = VK_FALSE;
        this->rasterizationInfo.depthBiasConstantFactor = 0.0f; // Optional
        this->rasterizationInfo.depthBiasClamp = 0.0f;          // Optional
        this->rasterizationInfo.depthBiasSlopeFactor = 0.0f;    // Optional
        this->rasterizationInfo.flags = 0;
        this->rasterizationInfo.pNext = nullptr;

        this->multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        this->multisampleInfo.sampleShadingEnable = VK_FALSE;
        this->multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        this->multisampleInfo.minSampleShading = 1.0f;          // Optional
        this->multisampleInfo.pSampleMask = nullptr;            // Optional
        this->multisampleInfo.alphaToCoverageEnable = VK_FALSE; // Optional
        this->multisampleInfo.alphaToOneEnable = VK_FALSE;      // Optional
        this->multisampleInfo.flags = 0;
        this->multisampleInfo.pNext = nullptr;

        this->colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        this->colorBlendAttachment.blendEnable = VK_FALSE;
        this->colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
        this->colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        this->colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
        this->colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
        this->colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        this->colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional

        this->colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        this->colorBlendInfo.logicOpEnable = VK_FALSE;
        this->colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
        this->colorBlendInfo.attachmentCount = 1;
        this->colorBlendInfo.pAttachments = &this->colorBlendAttachment;
        this->colorBlendInfo.blendConstants[0] = 0.0f; // Optional
        this->colorBlendInfo.blendConstants[1] = 0.0f; // Optional
        this->colorBlendInfo.blendConstants[2] = 0.0f; // Optional
        this->colorBlendInfo.blendConstants[3] = 0.0f; // Optional
        this->colorBlendInfo.flags = 0;
        this->colorBlendInfo.pNext = nullptr;

        this->depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        this->depthStencilInfo.depthTestEnable = VK_TRUE;
        this->depthStencilInfo.depthWriteEnable = VK_TRUE;
        this->depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        this->depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        this->depthStencilInfo.minDepthBounds = 0.0f; // Optional
        this->depthStencilInfo.maxDepthBounds = 1.0f; // Optional
        this->depthStencilInfo.stencilTestEnable = VK_FALSE;
        this->depthStencilInfo.front = {}; // Optional
        this->depthStencilInfo.back = {};  // Optional
        this->depthStencilInfo.flags = 0;
        this->depthStencilInfo.pNext = nullptr;

        this->dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        this->dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        this->dynamicStateInfo.pDynamicStates = this->dynamicStateEnables.data();
        this->dynamicStateInfo.dynamicStateCount =
            static_cast<uint32_t>(this->dynamicStateEnables.size());
        this->dynamicStateInfo.flags = 0;
        this->dynamicStateInfo.pNext = nullptr;

        this->pipelineLayout = VK_NULL_HANDLE;
        this->renderPass = VK_NULL_HANDLE;
        this->subpass = 0;

        this->vertexBindingDescriptions = {};
        this->vertexAttributeDescriptions = {};
        this->vertFilepath = "";
        this->fragFilepath = "";
    }

    GraphicPipeline::GraphicPipeline(
        Device &device,
        const GraphicPipelineConfigInfo &configInfo)
        : lveDevice{device}
    {
        createGraphicsPipeline(configInfo);
    }

    GraphicPipeline::~GraphicPipeline()
    {
        vkDestroyShaderModule(lveDevice.vkDevice(), vertShaderModule, nullptr);
        vkDestroyShaderModule(lveDevice.vkDevice(), fragShaderModule, nullptr);
        vkDestroyPipeline(lveDevice.vkDevice(), graphicPipeline, nullptr);
    }

    void GraphicPipeline::createGraphicsPipeline(const GraphicPipelineConfigInfo &configInfo)
    {
        assert(
            configInfo.pipelineLayout != VK_NULL_HANDLE &&
            "Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
        assert(
            configInfo.renderPass != VK_NULL_HANDLE &&
            "Cannot create graphics pipeline: no renderPass provided in configInfo");

        io::YamlConfig generalConfig{"config/general.yaml"};
        std::string shaderRoot = generalConfig.get<std::string>("shaderRoot") + "/";
        std::vector<char> vertCode = io::readBinaryFile(shaderRoot + configInfo.vertFilepath);
        std::vector<char> fragCode = io::readBinaryFile(shaderRoot + configInfo.fragFilepath);

        createShaderModule(lveDevice, vertCode, &vertShaderModule);
        createShaderModule(lveDevice, fragCode, &fragShaderModule);

        VkPipelineShaderStageCreateInfo shaderStages[2];
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertShaderModule;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;
        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        auto bindingDescriptions = configInfo.vertexBindingDescriptions;
        auto attributeDescriptions = configInfo.vertexAttributeDescriptions;
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexAttributeDescriptionCount =
            static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
        pipelineInfo.pViewportState = &configInfo.viewportInfo;
        pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
        pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
        pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
        pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
        pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

        pipelineInfo.layout = configInfo.pipelineLayout;
        pipelineInfo.renderPass = configInfo.renderPass;
        pipelineInfo.subpass = configInfo.subpass;

        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(
                lveDevice.vkDevice(),
                VK_NULL_HANDLE,
                1,
                &pipelineInfo,
                nullptr,
                &graphicPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline");
        }
    }
} // namespace lve
