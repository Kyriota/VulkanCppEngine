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
    class ComputeRTApp
    {
    public:
        static constexpr int INIT_WIDTH = 800;
        static constexpr int INIT_HEIGHT = 600;

        ComputeRTApp();
        ~ComputeRTApp();

        ComputeRTApp(const ComputeRTApp &) = delete;
        ComputeRTApp &operator=(const ComputeRTApp &) = delete;

        void run();

    private:
        void loadGameObjects();

        VkImageCreateInfo createScreenTextureInfo(VkFormat format, VkExtent2D extent);
        void createScreenTextureImageView();


        LveWindow lveWindow{INIT_WIDTH, INIT_HEIGHT, "Vulkan Compute Shader Raytracer"};
        LveDevice lveDevice{lveWindow};
        LveRenderer lveRenderer{lveWindow, lveDevice};

        LveImage screenTextureImage{
            lveDevice,
            createScreenTextureInfo(VK_FORMAT_R8G8B8A8_UNORM, lveWindow.getExtent()),
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

        // note: order of declarations matters because of destruction order
        std::unique_ptr<LveDescriptorPool> globalPool{};
        LveGameObject::Map gameObjects;
    };
} // namespace lve