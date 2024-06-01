#pragma once

#include "fluid_particle_system.hpp"
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
    class FluidSim2DApp
    {
    public:
        const std::string WINDOW_RESIZED_CALLBACK_NAME = "FluidSim2DApp";

        FluidSim2DApp();
        ~FluidSim2DApp();

        FluidSim2DApp(const FluidSim2DApp &) = delete;
        FluidSim2DApp &operator=(const FluidSim2DApp &) = delete;

        void run();

    private:
        LveWindow lveWindow{800, 600, "FluidSim2DApp"};
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

        LveImage screenTextureImage{lveDevice};
        VkFormat screenTextureFormat = VK_FORMAT_R8G8B8A8_UNORM;

        FluidParticleSystem fluidParticleSys{
            FluidParticleSystem::ParticleSysInitData{
                .particleCount = 128,
                .windowExtent = lveWindow.getExtent()}};

        void updateGlobalDescriptorSets(bool build = false);

        VkImageCreateInfo createScreenTextureInfo(VkFormat format, VkExtent2D extent);
        void createScreenTextureImageView();
        void recreateScreenTextureImage(VkExtent2D extent);

        void initParticleBuffer();
        void writeParticleBuffer();

        // Multi-threading
        std::atomic<bool> isRunning{true};
        void renderLoop();
    };
} // namespace lve