#pragma once

#include "lve/core/device.hpp"
#include "lve/core/frame_manager.hpp"
#include "lve/core/resource/descriptors.hpp"
#include "lve/core/resource/image.hpp"
#include "lve/core/window.hpp"
#include "lve/go/game_object.hpp"

// std
#include <memory>
#include <vector>

class RendererApp
{
public:
    static constexpr int INIT_WIDTH = 800;
    static constexpr int INIT_HEIGHT = 600;

    RendererApp();

    RendererApp(const RendererApp &) = delete;
    RendererApp &operator=(const RendererApp &) = delete;

    void run();

private:
    void loadGameObjects();

    lve::Window lveWindow{INIT_WIDTH, INIT_HEIGHT, "RendererApp"};
    lve::Device lveDevice{lveWindow};
    lve::FrameManager lveRenderer{lveWindow, lveDevice};

    // note: order of declarations matters because of destruction order
    std::unique_ptr<lve::DescriptorPool> globalPool{};
    std::vector<std::unique_ptr<lve::Buffer>> uboBuffers;
    std::unique_ptr<lve::DescriptorSetLayout> globalSetLayout;
    std::vector<VkDescriptorSet> globalDescriptorSets;
    lve::GameObject::Map gameObjects;

    void updateGlobalDescriptorSets();
};