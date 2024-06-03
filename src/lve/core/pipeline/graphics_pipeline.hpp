#pragma once

#include "lve/core/device.hpp"

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

    class GraphicPipeline
    {
    public:
        GraphicPipeline(
            Device &device,
            const GraphicPipelineConfigInfo &configInfo);
        ~GraphicPipeline();

        GraphicPipeline(const GraphicPipeline &) = delete;
        GraphicPipeline &operator=(const GraphicPipeline &) = delete;

        VkPipeline getPipeline() { return graphicPipeline; }

    private:
        void createGraphicsPipeline(const GraphicPipelineConfigInfo &configInfo);

        Device &lveDevice;
        VkPipeline graphicPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };
} // namespace lve
