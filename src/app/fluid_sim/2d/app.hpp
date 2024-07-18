#pragma once

#include "app/fluid_sim/2d/fluid_particle_system.hpp"
#include "lve/GO/geo/line.hpp"
#include "lve/core/device.hpp"
#include "lve/core/frame_manager.hpp"
#include "lve/core/pipeline/compute_pipeline.hpp"
#include "lve/core/pipeline/graphics_pipeline.hpp"
#include "lve/core/resource/descriptors.hpp"
#include "lve/core/resource/image.hpp"
#include "lve/core/resource/sampler_manager.hpp"
#include "lve/core/window.hpp"

// std
#include <atomic>
#include <memory>
#include <vector>

class FluidSim2DApp
{
public:
    const std::string WINDOW_RESIZED_CALLBACK_NAME = "FluidSim2DApp";

    FluidSim2DApp();

    FluidSim2DApp(const FluidSim2DApp &) = delete;
    FluidSim2DApp &operator=(const FluidSim2DApp &) = delete;

    void run();

private:
#ifdef NDEBUG
    const std::string APP_NAME = "FluidSim2DApp";
#else
    const std::string APP_NAME = "FluidSim2DApp (debug)";
#endif
    lve::Window lveWindow{128, 128, APP_NAME};
    lve::Device lveDevice{lveWindow};
    lve::FrameManager lveRenderer{lveWindow, lveDevice};
    VkExtent2D windowExtent = lveWindow.getExtent();

    // Frame rate
    struct FpsCounter
    {
        int frameCount = 0;
        std::chrono::_V2::system_clock::time_point startTime =
            std::chrono::high_resolution_clock::now();
    };
    float maxFrameTime = 1.0 / 30.0;
    FpsCounter fpsCounter;

    // GPU resources
    std::unique_ptr<lve::DescriptorPool> globalPool{};
    std::vector<std::unique_ptr<lve::Buffer>> uboBuffers;
    std::unique_ptr<lve::Buffer> particleBuffer;
    std::unique_ptr<lve::Buffer> neighborBuffer;
    std::unique_ptr<lve::DescriptorSetLayout> globalSetLayout;
    std::vector<VkDescriptorSet> globalDescriptorSets;
    lve::SamplerManager samplerManager{lveDevice};
    lve::GraphicPipeline screenTextureRenderPipeline{lveDevice};
    lve::GraphicPipeline lineRenderPipeline{lveDevice};
    lve::ComputePipeline fluidSimComputePipeline{lveDevice};

    lve::Image screenTextureImage{lveDevice};
    VkFormat screenTextureFormat = VK_FORMAT_R8G8B8A8_UNORM;

    FluidParticleSystem fluidParticleSys{"config/fluidSim2D.yaml", lveWindow.getExtent()};
    lve::LineCollection lineCollection{lveDevice, fluidParticleSys.getParticleCount()};

    void updateGlobalDescriptorSets(bool build = false);

    VkImageCreateInfo createScreenTextureInfo(VkFormat format, VkExtent2D extent);
    void createScreenTextureImageView();
    void recreateScreenTextureImage(VkExtent2D extent);

    void initParticleBuffer();
    void writeParticleBuffer();
    void drawDebugLines(VkCommandBuffer cmdBuffer);

    // Input
    void handleInput();

    // Multi-threading
    std::atomic<bool> isRunning{true};
    void renderLoop();
};