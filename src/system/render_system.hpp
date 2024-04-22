#pragma once

#include "../lve/lve_device.hpp"
#include "../lve/lve_frame_info.hpp"
#include "../lve/lve_game_object.hpp"
#include "../lve/lve_pipeline_graphics.hpp"

// std
#include <memory>
#include <vector>
#include <string>

namespace lve
{
    void renderGameObjects(FrameInfo &frameInfo, VkPipelineLayout graphicPipelineLayout, LveGraphicPipeline *graphicPipeline);

    void renderScreenTexture(FrameInfo &frameInfo, VkPipelineLayout graphicPipelineLayout, LveGraphicPipeline *graphicPipeline);

    class RenderSystem
    {
    public:
        RenderSystem(
            LveDevice &device,
            VkRenderPass renderPass,
            std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
            GraphicPipelineConfigInfo &graphicPipelineConfigInfo);
        ~RenderSystem();

        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        VkPipelineLayout getPipelineLayout() const { return graphicPipelineLayout; }
        LveGraphicPipeline *getPipeline() const { return lveGraphicPipeline.get(); }

    private:
        void createGraphicPipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
        void createGraphicPipeline(VkRenderPass renderPass, GraphicPipelineConfigInfo &graphicPipelineConfigInfo);

        LveDevice &lveDevice;
        std::unique_ptr<LveGraphicPipeline> lveGraphicPipeline;
        VkPipelineLayout graphicPipelineLayout;
    };
} // namespace lve
