#pragma once

#include "lve_device.hpp"
#include "lve_frame_info.hpp"
#include "lve_game_object.hpp"
#include "lve_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace lve
{
    class SimpleRenderSystem
    {
    public:
        SimpleRenderSystem(LveDevice &device, VkRenderPass renderPass);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem &) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

        void renderGameObjects(FrameInfo &frameInfo, std::vector<LveGameObject> &gameObjects);

    private:
        void createGraphicPipelineLayout();
        void createGraphicPipeline(VkRenderPass renderPass);

        LveDevice &lveDevice;
        std::unique_ptr<LveGraphicPipeline> lveGraphicPipeline;
        VkPipelineLayout graphicPipelineLayout;
    };
} // namespace lve
