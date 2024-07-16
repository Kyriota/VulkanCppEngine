#include "app/renderer/app.hpp"
#include "app/renderer/controller.hpp"

#include "lve/core/pipeline/graphics_pipeline.hpp"
#include "lve/core/resource/buffer.hpp"
#include "lve/core/resource/sampler_manager.hpp"
#include "lve/go/comp/camera.hpp"
#include "lve/util/file_io.hpp"

// libs
#include "include/glm.hpp"

// std
#include <cassert>
#include <chrono>
#include <cmath>
#include <stdexcept>
#include <string>

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
        lve::DescriptorPool::Builder(lveDevice)
            .setMaxSets(lve::SwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, lve::SwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
    loadGameObjects();
}

void RendererApp::run()
{
    uboBuffers.resize(lve::SwapChain::MAX_FRAMES_IN_FLIGHT);
    globalDescriptorSets.resize(lve::SwapChain::MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < uboBuffers.size(); i++)
    {
        uboBuffers[i] = std::make_unique<lve::Buffer>(
            lveDevice,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        uboBuffers[i]->map();
    }

    globalSetLayout =
        lve::DescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

    updateGlobalDescriptorSets();

    lve::GraphicPipelineConfigInfo graphicPipelineConfigInfo{};
    graphicPipelineConfigInfo.vertFilePath = "simple_shader.vert.spv";
    graphicPipelineConfigInfo.fragFilePath = "simple_shader.frag.spv";
    graphicPipelineConfigInfo.renderPass = lveRenderer.getSwapChainRenderPass();
    graphicPipelineConfigInfo.vertexBindingDescriptions =
        lve::Model::Vertex::getBindingDescriptions();
    graphicPipelineConfigInfo.vertexAttributeDescriptions =
        lve::Model::Vertex::getAttributeDescriptions();

    lve::GraphicPipeline simpleRenderPipeline{
        lveDevice,
        lve::GraphicPipelineLayoutConfigInfo{
                                             .descriptorSetLayouts = {globalSetLayout->getDescriptorSetLayout()},
                                             .pushConstantRanges =
                {{VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                  0,
                  sizeof(lve::SimplePushConstantData)}}
        },
        graphicPipelineConfigInfo
    };

    lve::Camera camera{};

    auto viewerObject = lve::GameObject::createGameObject();
    viewerObject.transform.translation.z = -2.5f;
    KeyboardMovementController cameraController{};

    auto currentTime = std::chrono::high_resolution_clock::now();
    while (!lveWindow.shouldClose())
    {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime =
            std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime)
                .count();
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
                simpleRenderPipeline.getPipelineLayout(),
                &simpleRenderPipeline
            );

            lveRenderer.endSwapChainRenderPass(commandBuffer);
            lveRenderer.endFrame();
        }
    }

    vkDeviceWaitIdle(lveDevice.vkDevice());
}

void RendererApp::loadGameObjects()
{
    lve::io::YamlConfig generalConfig{"config/general.yaml"};
    std::string modelRoot = generalConfig.get<std::string>("modelRoot") + "/";

    std::shared_ptr<lve::Model> lveModel =
        lve::Model::createModelFromFile(lveDevice, modelRoot + "flat_vase.obj");
    auto flatVase = lve::GameObject::createGameObject();
    flatVase.model = lveModel;
    flatVase.transform.translation = {-.5f, .5f, 0.f};
    flatVase.transform.scale = {3.f, 1.5f, 3.f};
    gameObjects.emplace(flatVase.getId(), std::move(flatVase));

    lveModel = lve::Model::createModelFromFile(lveDevice, modelRoot + "smooth_vase.obj");
    auto smoothVase = lve::GameObject::createGameObject();
    smoothVase.model = lveModel;
    smoothVase.transform.translation = {.5f, .5f, 0.f};
    smoothVase.transform.scale = {3.f, 1.5f, 3.f};
    gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

    lveModel = lve::Model::createModelFromFile(lveDevice, modelRoot + "quad.obj");
    auto floor = lve::GameObject::createGameObject();
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
        lve::DescriptorWriter writer{*globalSetLayout, *globalPool};
        writer.writeBuffer(0, &uboBufferInfo);

        writer.allocateDescriptorSet(globalDescriptorSets[i]);
        writer.overwrite(globalDescriptorSets[i]);
    }
}