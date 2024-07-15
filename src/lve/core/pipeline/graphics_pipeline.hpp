#pragma once

#include "lve/core/device.hpp"
#include "lve/go/game_object.hpp"
#include "lve/go/geo/line.hpp"

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

    struct GraphicPipelineLayoutConfigInfo
    {
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        std::vector<VkPushConstantRange> pushConstantRanges;
    };

    class GraphicPipeline
    {
    public:
        GraphicPipeline(Device &device) : lveDevice(device) {}
        GraphicPipeline(Device &device, VkRenderPass renderPass,
                        const GraphicPipelineLayoutConfigInfo &layoutConfigInfo,
                        const GraphicPipelineConfigInfo &pipelineConfigInfo);
        ~GraphicPipeline();

        GraphicPipeline(const GraphicPipeline &) = delete;
        GraphicPipeline &operator=(const GraphicPipeline &) = delete;

        GraphicPipeline(GraphicPipeline &&other) noexcept;
        GraphicPipeline &operator=(GraphicPipeline &&other);

        VkPipeline getPipeline() { return graphicPipeline; }
        VkPipelineLayout getPipelineLayout() { return graphicPipelineLayout; }

    private:
        void cleanUp();

        void createGraphicPipelineLayout(const GraphicPipelineLayoutConfigInfo &configInfo);
        void createGraphicsPipeline(VkRenderPass renderPass, const GraphicPipelineConfigInfo &configInfo);

        Device &lveDevice;
        VkPipeline graphicPipeline;
        VkPipelineLayout graphicPipelineLayout;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;

        bool initialized = false;
    };
    
    void renderGameObjects(
        VkCommandBuffer cmdBuffer,
        const VkDescriptorSet *pGlobalDescriptorSet,
        GameObject::Map &gameObjects,
        VkPipelineLayout graphicPipelineLayout,
        GraphicPipeline *graphicPipeline);

    void renderScreenTexture(
        VkCommandBuffer cmdBuffer,
        const VkDescriptorSet *pGlobalDescriptorSet,
        VkPipelineLayout graphicPipelineLayout,
        GraphicPipeline *graphicPipeline,
        VkExtent2D extent);

    void renderLines(
        VkCommandBuffer cmdBuffer,
        const VkDescriptorSet *pGlobalDescriptorSet,
        VkPipelineLayout graphicPipelineLayout,
        GraphicPipeline *graphicPipeline,
        LineCollection &lineCollection);
} // namespace lve
