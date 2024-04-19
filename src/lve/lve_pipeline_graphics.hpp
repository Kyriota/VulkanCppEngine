#pragma once

#include "lve_device.hpp"

// std
#include <string>
#include <vector>

namespace lve
{
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
} // namespace lve
