#include "renderer.hpp"

#include "keyboard_movement_controller.hpp"
#include "lve/lve_buffer.hpp"
#include "lve/lve_camera.hpp"
#include "lve/lve_sampler_manager.hpp"
#include "system/render_system.hpp"
#include "system/compute_system.hpp"

// libs
#include "include/glm.hpp"

// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>
#include <cmath>

namespace lve
{

    struct GlobalUbo
    {
        glm::mat4 projectionView{1.f};
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f}; // w is intensity
        glm::vec3 lightPosition{-1.f};
        alignas(16) glm::vec4 lightColor{1.f}; // w is light intensity
    };

    RendererApp::RendererApp()
    {
        globalPool =
            LveDescriptorPool::Builder(lveDevice)
                .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                .build();
        loadGameObjects();
    }

    void RendererApp::run()
    {
        uboBuffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        globalDescriptorSets.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < uboBuffers.size(); i++)
        {
            uboBuffers[i] = std::make_unique<LveBuffer>(
                lveDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        globalSetLayout =
            LveDescriptorSetLayout::Builder(lveDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .build();

        updateGlobalDescriptorSets();

        GraphicPipelineConfigInfo graphicPipelineConfigInfo{};
        graphicPipelineConfigInfo.vertFilepath = "build/shaders/simple_shader.vert.spv";
        graphicPipelineConfigInfo.fragFilepath = "build/shaders/simple_shader.frag.spv";
        graphicPipelineConfigInfo.vertexBindingDescriptions = LveModel::Vertex::getBindingDescriptions();
        graphicPipelineConfigInfo.vertexAttributeDescriptions = LveModel::Vertex::getAttributeDescriptions();

        RenderSystem simpleRenderSystem{
            lveDevice,
            lveRenderer.getSwapChainRenderPass(),
            {globalSetLayout->getDescriptorSetLayout()},
            graphicPipelineConfigInfo};

        LveCamera camera{};

        auto viewerObject = LveGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!lveWindow.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

            if (auto commandBuffer = lveRenderer.beginFrame())
            {
                int frameIndex = lveRenderer.getFrameIndex();

                // update
                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                lveRenderer.beginSwapChainRenderPass(commandBuffer);

                renderGameObjects(
                    commandBuffer,
                    &globalDescriptorSets[frameIndex],
                    gameObjects,
                    simpleRenderSystem.getPipelineLayout(),
                    simpleRenderSystem.getPipeline());

                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lveDevice.device());
    }

    void RendererApp::loadGameObjects()
    {
        std::shared_ptr<LveModel> lveModel =
            LveModel::createModelFromFile(lveDevice, "assets/models/flat_vase.obj");
        auto flatVase = LveGameObject::createGameObject();
        flatVase.model = lveModel;
        flatVase.transform.translation = {-.5f, .5f, 0.f};
        flatVase.transform.scale = {3.f, 1.5f, 3.f};
        gameObjects.emplace(flatVase.getId(), std::move(flatVase));

        lveModel = LveModel::createModelFromFile(lveDevice, "assets/models/smooth_vase.obj");
        auto smoothVase = LveGameObject::createGameObject();
        smoothVase.model = lveModel;
        smoothVase.transform.translation = {.5f, .5f, 0.f};
        smoothVase.transform.scale = {3.f, 1.5f, 3.f};
        gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

        lveModel = LveModel::createModelFromFile(lveDevice, "assets/models/quad.obj");
        auto floor = LveGameObject::createGameObject();
        floor.model = lveModel;
        floor.transform.translation = {0.f, .5f, 0.f};
        floor.transform.scale = {3.f, 1.f, 3.f};
        gameObjects.emplace(floor.getId(), std::move(floor));
    }

    void RendererApp::updateGlobalDescriptorSets()
    {
        for (int i = 0; i < globalDescriptorSets.size(); i++)
        {
            auto uboBufferInfo = uboBuffers[i]->descriptorInfo();
            LveDescriptorWriter writer{*globalSetLayout, *globalPool};
            writer.writeBuffer(0, &uboBufferInfo);

            writer.allocateDescriptorSet(globalDescriptorSets[i]);
            writer.overwrite(globalDescriptorSets[i]);
        }
    }
} // namespace lve
