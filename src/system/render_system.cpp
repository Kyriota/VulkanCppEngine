#include "render_system.hpp"
#include "lve/lve_pipeline_op.hpp"

// libs
#include "include/glm.hpp"

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
        initialized = true;
    }

    RenderSystem::~RenderSystem()
    {
        cleanUp();
    }

    RenderSystem::RenderSystem(RenderSystem &&other) noexcept
        : lveDevice{other.lveDevice},
          lveGraphicPipeline{std::move(other.lveGraphicPipeline)},
          graphicPipelineLayout{other.graphicPipelineLayout},
          initialized{other.initialized}
    {
        other.graphicPipelineLayout = nullptr;
    }

    RenderSystem &RenderSystem::operator=(RenderSystem &&other)
    {
        if (this->lveDevice.device() != other.lveDevice.device())
        {
            throw std::runtime_error("Moved RenderSystem objects must be on the same LveDevice");
        }

        if (this != &other)
        {
            // Clean up existing resources
            cleanUp();

            lveGraphicPipeline = std::move(other.lveGraphicPipeline);
            graphicPipelineLayout = other.graphicPipelineLayout;
            initialized = other.initialized;

            // Reset other object
            other.graphicPipelineLayout = nullptr;
        }

        return *this;
    }

    void RenderSystem::cleanUp()
    {
        if (initialized)
        {
            vkDestroyPipelineLayout(lveDevice.device(), graphicPipelineLayout, nullptr);
        }
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
