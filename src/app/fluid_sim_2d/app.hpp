#pragma once

// app
#include "mpm.hpp"
#include "gpu_resources/line_render_pipeline.hpp"
#include "gpu_resources/dot_render_pipeline.hpp"

// lve
#include "lve/GO/geo/line.hpp"
#include "lve/app/fps.hpp"
#include "lve/core/device.hpp"
#include "lve/core/frame_manager.hpp"
#include "lve/core/resource/descriptors.hpp"
#include "lve/core/resource/image.hpp"
#include "lve/core/resource/sampler_manager.hpp"
#include "lve/core/window.hpp"
#include "lve/path.hpp"

// std
#include <atomic>
#include <memory>
#include <vector>

namespace app::fluidsim
{
class App
{
public:
    const std::string WINDOW_RESIZED_CALLBACK_NAME = "FluidSim2DApp";

    App();

    App(const App &) = delete;
    App &operator=(const App &) = delete;

    void run();

protected:
#ifdef NDEBUG
    const std::string APP_NAME = "FluidSim2D";
#else
    const std::string APP_NAME = "FluidSim2D (debug)";
#endif
    lve::Window lveWindow{128, 128, APP_NAME};
    lve::Device lveDevice{lveWindow};
    lve::FrameManager lveFrameManager{lveWindow, lveDevice};

    lve::FpsManager fpsManager{30, 165};

    MPM fluidParticleSys{};

    DotRenderPipeline dotRenderPipeline = DotRenderPipeline(lveFrameManager, fluidParticleSys);
    // LineRenderPipeline lineRenderPipeline = LineRenderPipeline(lveFrameManager, fluidParticleSys);

    // Input
    void handleInput();

    // Multi-threading
    std::atomic<bool> isRunning{true};
    void renderLoop();
};
} // namespace app::fluidsim