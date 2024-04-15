#pragma once

#include "lve_device.hpp"

// std
#include <string>
#include <vector>

namespace lve
{

    void createShaderModule(LveDevice &lveDevice, const std::vector<char> &code, VkShaderModule *shaderModule);

    void bind(VkCommandBuffer commandBuffer, VkPipeline pipeline);

    // ========================== Graphic Pipeline ==========================

    struct GraphicPipelineConfigInfo
    {
        GraphicPipelineConfigInfo();
        GraphicPipelineConfigInfo(const GraphicPipelineConfigInfo &) = delete;
        GraphicPipelineConfigInfo &operator=(const GraphicPipelineConfigInfo &) = delete;

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;

        VkPipelineLayout pipelineLayout;
        VkRenderPass renderPass;
        uint32_t subpass;

        std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
        std::string vertFilepath;
        std::string fragFilepath;
    };

    class LveGraphicPipeline
    {
    public:
        LveGraphicPipeline(
            LveDevice &device,
            const GraphicPipelineConfigInfo &configInfo);
        ~LveGraphicPipeline();

        LveGraphicPipeline(const LveGraphicPipeline &) = delete;
        LveGraphicPipeline &operator=(const LveGraphicPipeline &) = delete;

        VkPipeline getPipeline() { return graphicPipeline; }

    private:
        void createGraphicsPipeline(const GraphicPipelineConfigInfo &configInfo);

        LveDevice &lveDevice;
        VkPipeline graphicPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };

    // ========================== Compute Pipeline ==========================

    struct ComputePipelineConfigInfo
    {
        ComputePipelineConfigInfo() = default;
        ComputePipelineConfigInfo(const ComputePipelineConfigInfo &) = delete;
        ComputePipelineConfigInfo &operator=(const ComputePipelineConfigInfo &) = delete;

        VkPipelineLayout pipelineLayout = nullptr;
    };

    class LveComputePipeline
    {
    public:
        LveComputePipeline(LveDevice &device, const std::string &compFilepath, const ComputePipelineConfigInfo &configInfo);
        ~LveComputePipeline();

        LveComputePipeline(const LveComputePipeline &) = delete;
        LveComputePipeline &operator=(const LveComputePipeline &) = delete;

        VkPipeline getPipeline() { return computePipeline; }

    private:
        void createComputePipeline(const std::string &compFilepath, const ComputePipelineConfigInfo &configInfo);

        LveDevice &lveDevice;
        VkPipeline computePipeline;
        VkShaderModule compShaderModule;
    };
} // namespace lve
