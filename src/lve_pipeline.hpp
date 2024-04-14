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
        GraphicPipelineConfigInfo() = default;
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
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class LveGraphicPipeline
    {
    public:
        LveGraphicPipeline(
            LveDevice &device,
            const std::string &vertFilepath,
            const std::string &fragFilepath,
            const GraphicPipelineConfigInfo &configInfo);
        ~LveGraphicPipeline();

        LveGraphicPipeline(const LveGraphicPipeline &) = delete;
        LveGraphicPipeline &operator=(const LveGraphicPipeline &) = delete;

        void bind(VkCommandBuffer commandBuffer);

        static void defaultPipelineConfigInfo(GraphicPipelineConfigInfo &configInfo);

        VkPipeline getPipeline() { return graphicPipeline; }

    private:
        void createGraphicsPipeline(
            const std::string &vertFilepath,
            const std::string &fragFilepath,
            const GraphicPipelineConfigInfo &configInfo);

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
