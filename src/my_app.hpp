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
    struct ParticleBuffer
    {
        int numParticles;
        std::vector<glm::vec2> positions;
    };

    class MyApp
    {
    public:
        static const std::string WINDOW_RESIZED_CALLBACK_NAME;
        static constexpr int INIT_WIDTH = 800;
        static constexpr int INIT_HEIGHT = 600;

        MyApp();
        ~MyApp();

        MyApp(const MyApp &) = delete;
        MyApp &operator=(const MyApp &) = delete;

        void run();

    private:
        void loadGameObjects();

        LveWindow lveWindow{INIT_WIDTH, INIT_HEIGHT, "Vulkan Compute Shader Raytracer"};
        LveDevice lveDevice{lveWindow};
        LveRenderer lveRenderer{lveWindow, lveDevice};

        // note: order of declarations matters because of destruction order
        std::unique_ptr<LveDescriptorPool> globalPool{};
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers;
        std::unique_ptr<LveBuffer> particleBuffer;
        std::unique_ptr<LveDescriptorSetLayout> globalSetLayout;
        std::vector<VkDescriptorSet> globalDescriptorSets;
        LveGameObject::Map gameObjects;

        void updateGlobalDescriptorSets(bool build=false);

        LveImage screenTextureImage{lveDevice};
        VkFormat screenTextureFormat = VK_FORMAT_R8G8B8A8_UNORM;

        VkImageCreateInfo createScreenTextureInfo(VkFormat format, VkExtent2D extent);
        void createScreenTextureImageView();
        void recreateScreenTextureImage(VkExtent2D extent);

        const int PARTICLE_COUNT = 32;
        ParticleBuffer initParticleBuffer(glm::vec2 startPoint, float stride, float maxWidth);
    };
} // namespace lve