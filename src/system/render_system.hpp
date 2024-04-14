#pragma once

#include "../lve/lve_device.hpp"
#include "../lve/lve_frame_info.hpp"
#include "../lve/lve_game_object.hpp"
#include "../lve/lve_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace lve
{
    class RenderSystem
    {
    public:
        RenderSystem(LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~RenderSystem();

        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        void renderGameObjects(FrameInfo &frameInfo);

    private:
        void createGraphicPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createGraphicPipeline(VkRenderPass renderPass);

        LveDevice &lveDevice;
        std::unique_ptr<LveGraphicPipeline> lveGraphicPipeline;
        VkPipelineLayout graphicPipelineLayout;
    };
} // namespace lve
