#include "simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace lve
{

    struct SimplePushConstantData
    {
        glm::mat4 transform{1.f};
        glm::mat4 normalMatrix{1.f};
    };

    SimpleRenderSystem::SimpleRenderSystem(LveDevice &device, VkRenderPass renderPass)
        : lveDevice{device}
    {
        createGraphicPipelineLayout();
        createGraphicPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem()
    {
        vkDestroyPipelineLayout(lveDevice.device(), graphicPipelineLayout, nullptr);
    }

    void SimpleRenderSystem::renderGameObjects(
        FrameInfo &frameInfo, std::vector<LveGameObject> &gameObjects)
    {
        bind(frameInfo.commandBuffer, lveGraphicPipeline->getPipeline());

        auto projectionView = frameInfo.camera.getProjection() * frameInfo.camera.getView();

        for (auto &obj : gameObjects)
        {
            SimplePushConstantData push{};
            auto modelMatrix = obj.transform.mat4();
            push.transform = projectionView * modelMatrix;
            push.normalMatrix = obj.transform.normalMatrix();

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                graphicPipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);
            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }

    void SimpleRenderSystem::createGraphicPipelineLayout()
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &graphicPipelineLayout) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void SimpleRenderSystem::createGraphicPipeline(VkRenderPass renderPass)
    {
        assert(graphicPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        GraphicPipelineConfigInfo graphicPipelineConfig{};
        LveGraphicPipeline::defaultPipelineConfigInfo(graphicPipelineConfig);
        graphicPipelineConfig.renderPass = renderPass;
        graphicPipelineConfig.pipelineLayout = graphicPipelineLayout;
        lveGraphicPipeline = std::make_unique<LveGraphicPipeline>(
            lveDevice,
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            graphicPipelineConfig);
    }

} // namespace lve
