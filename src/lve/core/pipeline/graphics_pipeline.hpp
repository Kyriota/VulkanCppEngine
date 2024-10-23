#pragma once

// lve
#include "lve/GO/game_object.hpp"
#include "lve/GO/geo/line.hpp"
#include "lve/core/device.hpp"
#include "lve/core/pipeline/pipeline_base.hpp"

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

class GraphicPipeline : public Pipeline
{
public:
    GraphicPipeline(Device &device) : Pipeline(device) {}
    GraphicPipeline(
        Device &device,
        const GraphicPipelineLayoutConfigInfo &layoutConfigInfo,
        const GraphicPipelineConfigInfo &pipelineConfigInfo);

    void bind(VkCommandBuffer commandBuffer) override;

private:
    void createGraphicPipelineLayout(const GraphicPipelineLayoutConfigInfo &configInfo);
    void createGraphicsPipeline(const GraphicPipelineConfigInfo &configInfo);
};

void renderGameObjects(
    VkCommandBuffer cmdBuffer,
    const VkDescriptorSet *pDescriptorSet,
    GameObject::Map &gameObjects,
    VkPipelineLayout graphicPipelineLayout,
    GraphicPipeline *graphicPipeline);

void renderScreenTexture(
    VkCommandBuffer cmdBuffer,
    const VkDescriptorSet *pDescriptorSet,
    VkPipelineLayout graphicPipelineLayout,
    GraphicPipeline *graphicPipeline,
    VkExtent2D extent);

void renderLines(
    VkCommandBuffer cmdBuffer, GraphicPipeline *graphicPipeline, LineCollection &lineCollection);
} // namespace lve
