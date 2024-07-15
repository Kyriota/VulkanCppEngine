#pragma once

#include "lve/core/device.hpp"
#include "lve/core/pipeline/pipeline.hpp"
#include "lve/go/game_object.hpp"
#include "lve/go/geo/line.hpp"

// std
#include <string>
#include <vector>

namespace lve
{
    struct SimplePushConstantData
    {
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
    };

    struct ScreenExtentPushConstantData
    {
        glm::vec2 screenExtent;
    };

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

        VkRenderPass renderPass;
        uint32_t subpass;

        std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
        std::string vertFilePath;
        std::string fragFilePath;
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
        GraphicPipeline(Device &device,
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
        void createGraphicsPipeline(const GraphicPipelineConfigInfo &configInfo);

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
