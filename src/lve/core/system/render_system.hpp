#pragma once

#include "lve/core/device.hpp"
#include "lve/core/game_object.hpp"
#include "lve/core/pipeline/graphics_pipeline.hpp"

// std
#include <memory>
#include <vector>
#include <string>

namespace lve
{
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

    class RenderSystem
    {
    public:
        RenderSystem(
            Device &device,
            VkRenderPass renderPass,
            std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
            GraphicPipelineConfigInfo &graphicPipelineConfigInfo);
        RenderSystem(Device &device) : lveDevice(device) {}
        ~RenderSystem();

        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        RenderSystem(RenderSystem &&other) noexcept;
        RenderSystem &operator=(RenderSystem &&other);

        VkPipelineLayout getPipelineLayout() const { return graphicPipelineLayout; }
        GraphicPipeline *getPipeline() const { return lveGraphicPipeline.get(); }

    private:
        void cleanUp();

        void createGraphicPipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
        void createGraphicPipeline(VkRenderPass renderPass, GraphicPipelineConfigInfo &graphicPipelineConfigInfo);

        Device &lveDevice;
        std::unique_ptr<GraphicPipeline> lveGraphicPipeline;
        VkPipelineLayout graphicPipelineLayout;
        
        bool initialized = false;
    };
} // namespace lve
