#include "app.hpp"

// lve
#include "lve/core/resource/buffer.hpp"
#include "lve/core/resource/sampler_manager.hpp"
#include "lve/path.hpp"
#include "lve/util/config.hpp"
#include "lve/util/file_io.hpp"
#include "lve/util/math.hpp"

// libs
#include "include/glm.hpp"

// std
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

namespace app::fluidsim
{
App::App()
{
    // resize window according to config
    std::vector<int> windowSize = lve::ConfigManager::getConfig(lve::path::config::FLUID_SIM_2D)
                                      .get<std::vector<int>>("windowSize");
    lveWindow.resize(windowSize[0], windowSize[1]);

    // // register callback functions for window resize
    // lveFrameManager.registerSwapChainResizedCallback(
    //     WINDOW_RESIZED_CALLBACK_NAME, [this](VkExtent2D extent) {
    //     });
}

void App::run()
{
    std::thread renderThread(&App::renderLoop, this);

    lveWindow.mainThreadGlfwEventLoop();

    isRunning = false;
    renderThread.join();

    vkDeviceWaitIdle(lveDevice.vkDevice());
}

void App::renderLoop()
{
    fpsManager.renderStart();
    while (isRunning)
    {
        double frameDuration = fpsManager.step([this](int frameCountInLastSecond) {
            lveWindow.setTitle(APP_NAME + " (FPS: " + std::to_string(frameCountInLastSecond) + ")");
        });

        if (VkCommandBuffer commandBuffer = lveFrameManager.beginFrame())
        {
            handleInput();

            // fluid particle system
            for (int i = 0; i < 10; i++)
                fluidParticleSys.substep(1e-4f);

            // render
            lveFrameManager.beginSwapChainRenderPass(commandBuffer);
            dotRenderPipeline.render(commandBuffer);
            // if (fluidParticleSys.isDebugLineOn())
            //     lineRenderPipeline.render(commandBuffer);

            lveFrameManager.endSwapChainRenderPass(commandBuffer);
            lveFrameManager.endFrame();
        }

        fpsManager.fpsLimitBusyWait();
    }
}
} // namespace app::fluidsim