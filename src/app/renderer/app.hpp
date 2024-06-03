#pragma once

#include "lve/core/resource/descriptors.hpp"
#include "lve/core/resource/image.hpp"
#include "lve/core/device.hpp"
#include "lve/core/game_object.hpp"
#include "lve/core/frame_manager.hpp"
#include "lve/core/window.hpp"

// std
#include <memory>
#include <vector>

namespace lve
{
    class RendererApp
    {
    public:
        static constexpr int INIT_WIDTH = 800;
        static constexpr int INIT_HEIGHT = 600;

        RendererApp();

        RendererApp(const RendererApp &) = delete;
        RendererApp &operator=(const RendererApp &) = delete;

        void run();

    private:
        void loadGameObjects();

        Window lveWindow{INIT_WIDTH, INIT_HEIGHT, "RendererApp"};
        Device lveDevice{lveWindow};
        FrameManager lveRenderer{lveWindow, lveDevice};

        // note: order of declarations matters because of destruction order
        std::unique_ptr<DescriptorPool> globalPool{};
        std::vector<std::unique_ptr<Buffer>> uboBuffers;
        std::unique_ptr<DescriptorSetLayout> globalSetLayout;
        std::vector<VkDescriptorSet> globalDescriptorSets;
        GameObject::Map gameObjects;

        void updateGlobalDescriptorSets();
    };
} // namespace lve