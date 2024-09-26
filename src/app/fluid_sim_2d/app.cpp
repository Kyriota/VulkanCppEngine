#include "app.hpp"

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
#include <cmath>
#include <iostream>
#include <thread>

namespace app::fluidsim2d
{
App::App()
{
    // resize window according to config
    std::vector<int> windowSize = lve::ConfigManager::getConfig(lve::path::config::FLUID_SIM_2D)
                                      .get<std::vector<int>>("windowSize");
    lveWindow.resize(windowSize[0], windowSize[1]);

    // register callback functions for window resize
    lveFrameManager.registerSwapChainResizedCallback(
        WINDOW_RESIZED_CALLBACK_NAME, [this](VkExtent2D extent) {
            fluidParticleSys.updateWindowExtent(extent);
        });
}

void App::run()
{
    std::thread renderThread(&App::renderLoop, this);

    lveWindow.mainThreadGlfwEventLoop();

    isRunning = false;
    renderThread.join();

    vkDeviceWaitIdle(lveDevice.vkDevice());
}

void App::handleInput()
{
    if (lveWindow.input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) ||
        lveWindow.input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
    {
        double mouseX, mouseY;
        lveWindow.input.getMousePosition(mouseX, mouseY);
        glm::vec2 mousePos = {static_cast<float>(mouseX), static_cast<float>(mouseY)};
        fluidParticleSys.setRangeForcePos(
            lveWindow.input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT), mousePos);
    }

    lveWindow.input.oneTimeKeyUse(GLFW_KEY_R, [this] {
        fluidParticleSys.reloadConfigParam();
        std::cout << "Reloaded config parameters" << std::endl;
    });
    lveWindow.input.oneTimeKeyUse(GLFW_KEY_SPACE, [this] {
        fluidParticleSys.togglePause();
    });
    lveWindow.input.oneTimeKeyUse(GLFW_KEY_F, [this] {
        fluidParticleSys.renderPausedNextFrame();
    });

    lveWindow.input.oneTimeKeyUse(GLFW_KEY_V, [this] {
        fluidParticleSys.toggleDebugLine();
    });
    lveWindow.input.oneTimeKeyUse(GLFW_KEY_1, [this] {
        fluidParticleSys.setDebugLineType(FluidParticleSystem::VELOCITY);
    });
    lveWindow.input.oneTimeKeyUse(GLFW_KEY_2, [this] {
        fluidParticleSys.setDebugLineType(FluidParticleSystem::PRESSURE_FORCE);
    });
    lveWindow.input.oneTimeKeyUse(GLFW_KEY_3, [this] {
        fluidParticleSys.setDebugLineType(FluidParticleSystem::EXTERNAL_FORCE);
    });

    lveWindow.input.oneTimeKeyUse(GLFW_KEY_N, [this] {
        fluidParticleSys.toggleNeighborView();
    });
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
            fluidParticleSys.updateParticleData(frameDuration);

            // render
            lveFrameManager.beginSwapChainRenderPass(commandBuffer);
            dotRenderPipeline.render(commandBuffer);
            if (fluidParticleSys.isDebugLineOn())
                lineRenderPipeline.render(commandBuffer);

            lveFrameManager.endSwapChainRenderPass(commandBuffer);
            lveFrameManager.endFrame();
        }

        fpsManager.fpsLimitBusyWait();
    }
}
} // namespace app::fluidsim2d