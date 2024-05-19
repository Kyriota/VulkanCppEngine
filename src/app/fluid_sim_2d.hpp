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
        unsigned int numParticles;
        std::vector<glm::vec2> positions;
        std::vector<glm::vec2> velocities;
    };

    class FluidSim2DApp
    {
    public:
        static const std::string WINDOW_RESIZED_CALLBACK_NAME;
        static constexpr int INIT_WIDTH = 800;
        static constexpr int INIT_HEIGHT = 600;

        FluidSim2DApp();
        ~FluidSim2DApp();

        FluidSim2DApp(const FluidSim2DApp &) = delete;
        FluidSim2DApp &operator=(const FluidSim2DApp &) = delete;

        void run();

    private:
        LveWindow lveWindow{INIT_WIDTH, INIT_HEIGHT, "FluidSim2DApp"};
        LveDevice lveDevice{lveWindow};
        LveRenderer lveRenderer{lveWindow, lveDevice};

        std::unique_ptr<LveDescriptorPool> globalPool{};
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers;
        std::unique_ptr<LveBuffer> particleBuffer;
        std::unique_ptr<LveDescriptorSetLayout> globalSetLayout;
        std::vector<VkDescriptorSet> globalDescriptorSets;

        ParticleBuffer particleBufferData;

        void updateGlobalDescriptorSets(bool build=false);

        LveImage screenTextureImage{lveDevice};
        VkFormat screenTextureFormat = VK_FORMAT_R8G8B8A8_UNORM;

        VkImageCreateInfo createScreenTextureInfo(VkFormat format, VkExtent2D extent);
        void createScreenTextureImageView();
        void recreateScreenTextureImage(VkExtent2D extent);

        const unsigned int PARTICLE_COUNT = 32;
        ParticleBuffer initParticleBufferData(glm::vec2 startPoint, float stride, float maxWidth);
        void initParticleBuffer();
        void writeParticleBuffer();
        void updateParticleBufferData(float deltaTime);
    };
} // namespace lve