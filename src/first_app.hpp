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
    class FirstApp
    {
    public:
        static constexpr int INIT_WIDTH = 800;
        static constexpr int INIT_HEIGHT = 600;

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp &) = delete;
        FirstApp &operator=(const FirstApp &) = delete;

        void run();

    private:
        void loadGameObjects();

        LveWindow lveWindow{INIT_WIDTH, INIT_HEIGHT, "Vulkan Tutorial"};
        LveDevice lveDevice{lveWindow};
        LveRenderer lveRenderer{lveWindow, lveDevice};

        LveImageConfig screenTextureConfig{
            INIT_WIDTH,
            INIT_HEIGHT,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};
        LveImage screenTextureImg{
            lveDevice,
            screenTextureConfig};

        // note: order of declarations matters because of destruction order
        std::unique_ptr<LveDescriptorPool> globalPool{};
        LveGameObject::Map gameObjects;
    };
} // namespace lve