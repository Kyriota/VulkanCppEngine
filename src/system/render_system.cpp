#include "render_system.hpp"
#include "lve/lve_pipeline_op.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>
#include <string>

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

    void renderGameObjects(
        VkCommandBuffer cmdBuffer,
        const VkDescriptorSet *pGlobalDescriptorSet,
        LveGameObject::Map &gameObjects,
        VkPipelineLayout graphicPipelineLayout,
        LveGraphicPipeline *graphicPipeline)
    {
        bind(cmdBuffer, graphicPipeline->getPipeline());

        vkCmdBindDescriptorSets(
            cmdBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            graphicPipelineLayout,
            0,
            1,
            pGlobalDescriptorSet,
            0,
            nullptr);

        for (auto &kv : gameObjects)
        {
            auto &obj = kv.second;
            if (obj.model == nullptr)
                continue;
            SimplePushConstantData push{};
            push.modelMatrix = obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();

            vkCmdPushConstants(
                cmdBuffer,
                graphicPipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);
            obj.model->bind(cmdBuffer);
            obj.model->draw(cmdBuffer);
        }
    }

    void renderScreenTexture(
        VkCommandBuffer cmdBuffer,
        const VkDescriptorSet *pGlobalDescriptorSet,
        VkPipelineLayout graphicPipelineLayout,
        LveGraphicPipeline *graphicPipeline,
        VkExtent2D extent)
    {
        bind(cmdBuffer, graphicPipeline->getPipeline());

        vkCmdBindDescriptorSets(
            cmdBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            graphicPipelineLayout,
            0,
            1,
            pGlobalDescriptorSet,
            0,
            nullptr);

        ScreenExtentPushConstantData push{};
        push.screenExtent = glm::vec2(extent.width, extent.height);
        
        vkCmdPushConstants(
            cmdBuffer,
            graphicPipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(ScreenExtentPushConstantData),
            &push);

        vkCmdDraw(cmdBuffer, 6, 1, 0, 0);
    }

    RenderSystem::RenderSystem(
        LveDevice &device,
        VkRenderPass renderPass,
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
        GraphicPipelineConfigInfo &graphicPipelineConfigInfo)
        : lveDevice{device}
    {
        createGraphicPipelineLayout(descriptorSetLayouts);
        createGraphicPipeline(renderPass, graphicPipelineConfigInfo);
    }

    RenderSystem::~RenderSystem()
    {
        vkDestroyPipelineLayout(lveDevice.device(), graphicPipelineLayout, nullptr);
    }

    void RenderSystem::createGraphicPipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &graphicPipelineLayout) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void RenderSystem::createGraphicPipeline(VkRenderPass renderPass, GraphicPipelineConfigInfo &graphicPipelineConfigInfo)
    {
        assert(graphicPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        graphicPipelineConfigInfo.renderPass = renderPass;
        graphicPipelineConfigInfo.pipelineLayout = graphicPipelineLayout;
        lveGraphicPipeline = std::make_unique<LveGraphicPipeline>(
            lveDevice,
            graphicPipelineConfigInfo);
    }

} // namespace lve
