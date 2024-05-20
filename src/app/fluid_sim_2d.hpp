#pragma once

#include "lve/lve_descriptors.hpp"
#include "lve/lve_device.hpp"
#include "lve/lve_game_object.hpp"
#include "lve/lve_renderer.hpp"
#include "lve/lve_window.hpp"
#include "lve/lve_image.hpp"
#include "system/render_system.hpp"
#include "system/compute_system.hpp"

// std
#include <memory>
#include <vector>
#include <atomic>

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
        VkExtent2D windowExtent = lveWindow.getExtent();

        // GPU resources
        std::unique_ptr<LveDescriptorPool> globalPool{};
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers;
        std::unique_ptr<LveBuffer> particleBuffer;
        std::unique_ptr<LveDescriptorSetLayout> globalSetLayout;
        std::vector<VkDescriptorSet> globalDescriptorSets;
        RenderSystem screenTextureRenderSystem{lveDevice};
        ComputeSystem fluidSimComputeSystem{lveDevice};

        // Fluid simulation data
        ParticleBuffer particleBufferData;
        float smoothRadius = 2.0f;
        float collisionDamping = 1.0f;
        float targetDensity = 1.0f;

        LveImage screenTextureImage{lveDevice};
        VkFormat screenTextureFormat = VK_FORMAT_R8G8B8A8_UNORM;

        void updateGlobalDescriptorSets(bool build=false);

        VkImageCreateInfo createScreenTextureInfo(VkFormat format, VkExtent2D extent);
        void createScreenTextureImageView();
        void recreateScreenTextureImage(VkExtent2D extent);

        const unsigned int PARTICLE_COUNT = 32;
        ParticleBuffer initParticleBufferData(glm::vec2 startPoint, float stride, float maxWidth);
        void initParticleBuffer();
        void writeParticleBuffer();
        void updateParticleBufferData(float deltaTime);
        void handleBoundaryCollision();

        // Multi-threading
        std::atomic<bool> isRunning{true};
        void renderLoop();
    };
} // namespace lve