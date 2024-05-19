#pragma once

#include "lve/lve_descriptors.hpp"
#include "lve/lve_device.hpp"
#include "lve/lve_game_object.hpp"
#include "lve/lve_renderer.hpp"
#include "lve/lve_window.hpp"
#include "lve/lve_image.hpp"

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

        LveWindow lveWindow{INIT_WIDTH, INIT_HEIGHT, "RendererApp"};
        LveDevice lveDevice{lveWindow};
        LveRenderer lveRenderer{lveWindow, lveDevice};

        // note: order of declarations matters because of destruction order
        std::unique_ptr<LveDescriptorPool> globalPool{};
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers;
        std::unique_ptr<LveDescriptorSetLayout> globalSetLayout;
        std::vector<VkDescriptorSet> globalDescriptorSets;
        LveGameObject::Map gameObjects;

        void updateGlobalDescriptorSets();
    };
} // namespace lve