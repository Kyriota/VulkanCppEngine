#pragma once

#include "app/fluid_sim_2d/fluid_particle_system.hpp"
#include "app/fluid_sim_2d/gpu_resources.hpp"
#include "lve/GO/geo/line.hpp"
#include "lve/core/device.hpp"
#include "lve/core/frame_manager.hpp"
#include "lve/core/resource/descriptors.hpp"
#include "lve/core/resource/image.hpp"
#include "lve/core/resource/sampler_manager.hpp"
#include "lve/core/window.hpp"
#include "lve/path.hpp"
#include "lve/app/fps.hpp"

// std
#include <atomic>
#include <memory>
#include <vector>

namespace fluidsim2d
{
class App
{
public:
    const std::string WINDOW_RESIZED_CALLBACK_NAME = "FluidSim2DApp";

    App();

    App(const App &) = delete;
    App &operator=(const App &) = delete;

    void run();

private:
#ifdef NDEBUG
    const std::string APP_NAME = "FluidSim2D";
#else
    const std::string APP_NAME = "FluidSim2D (debug)";
#endif
    // TODO:
    //  - Format all files for namespace indentation
    //  - Separate global ubo etc. from gpu resources
    //  - Separate private and public into more detailed sections
    lve::Window lveWindow{128, 128, APP_NAME};
    lve::Device lveDevice{lveWindow};
    lve::FrameManager lveFrameManager{lveWindow, lveDevice};

    lve::FpsManager fpsManager{30, 165};

    FluidParticleSystem fluidParticleSys{lveWindow.getExtent()};

    GpuResources gpuResources{lveFrameManager, fluidParticleSys};

    // Input
    void handleInput();

    // Multi-threading
    std::atomic<bool> isRunning{true};
    void renderLoop();
};
} // namespace fluidsim2d