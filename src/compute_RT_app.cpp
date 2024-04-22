#include "compute_RT_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "lve/lve_buffer.hpp"
#include "lve/lve_camera.hpp"
#include "lve/lve_sampler_manager.hpp"
#include "system/render_system.hpp"
#include "system/compute_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

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

    ComputeRTApp::ComputeRTApp()
    {
        globalPool =
            LveDescriptorPool::Builder(lveDevice)
                .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT * 3)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                .build();
        loadGameObjects();

        createScreenTextureImageView();
    }

    ComputeRTApp::~ComputeRTApp()
    {
        LveSamplerManager::clearSamplers();
    }

    void ComputeRTApp::run()
    {
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
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

        auto globalSetLayout =
            LveDescriptorSetLayout::Builder(lveDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
                .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        VkDescriptorImageInfo screenTextureDescriptorInfo = screenTextureImage.getDescriptorImageInfo(
            0, LveSamplerManager::getSampler({SamplerType::DEFAULT, lveDevice.device()}));
        for (int i = 0; i < globalDescriptorSets.size(); i++)
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            LveDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &screenTextureDescriptorInfo) // combined image sampler
                .writeImage(2, &screenTextureDescriptorInfo) // storage image
                .build(globalDescriptorSets[i]);
        }

        /*
        GraphicPipelineConfigInfo graphicPipelineConfigInfo{};
        graphicPipelineConfigInfo.vertFilepath = "shaders/simple_shader.vert.spv";
        graphicPipelineConfigInfo.fragFilepath = "shaders/simple_shader.frag.spv";
        graphicPipelineConfigInfo.vertexBindingDescriptions = LveModel::Vertex::getBindingDescriptions();
        graphicPipelineConfigInfo.vertexAttributeDescriptions = LveModel::Vertex::getAttributeDescriptions();

        RenderSystem simpleRenderSystem{
            lveDevice,
            lveRenderer.getSwapChainRenderPass(),
            {globalSetLayout->getDescriptorSetLayout()},
            graphicPipelineConfigInfo};
        */

        GraphicPipelineConfigInfo screenTexturePipelineConfigInfo{};
        screenTexturePipelineConfigInfo.vertFilepath = "shaders/screen_texture_shader.vert.spv";
        screenTexturePipelineConfigInfo.fragFilepath = "shaders/screen_texture_shader.frag.spv";

        RenderSystem screenTextureRenderSystem{
            lveDevice,
            lveRenderer.getSwapChainRenderPass(),
            {globalSetLayout->getDescriptorSetLayout()},
            screenTexturePipelineConfigInfo};

        ComputeSystem simpleComputeSystem{
            lveDevice,
            {globalSetLayout->getDescriptorSetLayout()},
            "shaders/my_compute_shader.comp.spv"};

        LveCamera camera{};

        // auto viewerObject = LveGameObject::createGameObject();
        // viewerObject.transform.translation.z = -2.5f;
        // KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!lveWindow.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            // cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
            // camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            // float aspect = lveRenderer.getAspectRatio();
            // camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

            if (auto commandBuffer = lveRenderer.beginFrame())
            {
                int frameIndex = lveRenderer.getFrameIndex();
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    gameObjects};

                // update
                // GlobalUbo ubo{};
                // ubo.projectionView = camera.getProjection() * camera.getView();
                // uboBuffers[frameIndex]->writeToBuffer(&ubo);
                // uboBuffers[frameIndex]->flush();

                VkExtent2D extent = lveWindow.getExtent();
                simpleComputeSystem.dispatchComputePipeline(
                    frameInfo,
                    static_cast<int>(std::ceil(extent.width / 8.f)),
                    static_cast<int>(std::ceil(extent.height / 8.f)));

                // render
                lveRenderer.beginSwapChainRenderPass(commandBuffer);

                // renderGameObjects(frameInfo, simpleRenderSystem.getPipelineLayout(), simpleRenderSystem.getPipeline());
                renderScreenTexture(frameInfo, screenTextureRenderSystem.getPipelineLayout(), screenTextureRenderSystem.getPipeline());

                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lveDevice.device());
    }

    void ComputeRTApp::loadGameObjects()
    {
        std::shared_ptr<LveModel> lveModel =
            LveModel::createModelFromFile(lveDevice, "models/flat_vase.obj");
        auto flatVase = LveGameObject::createGameObject();
        flatVase.model = lveModel;
        flatVase.transform.translation = {-.5f, .5f, 0.f};
        flatVase.transform.scale = {3.f, 1.5f, 3.f};
        gameObjects.emplace(flatVase.getId(), std::move(flatVase));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/smooth_vase.obj");
        auto smoothVase = LveGameObject::createGameObject();
        smoothVase.model = lveModel;
        smoothVase.transform.translation = {.5f, .5f, 0.f};
        smoothVase.transform.scale = {3.f, 1.5f, 3.f};
        gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/quad.obj");
        auto floor = LveGameObject::createGameObject();
        floor.model = lveModel;
        floor.transform.translation = {0.f, .5f, 0.f};
        floor.transform.scale = {3.f, 1.f, 3.f};
        gameObjects.emplace(floor.getId(), std::move(floor));
    }

    VkImageCreateInfo ComputeRTApp::createScreenTextureInfo(VkFormat format, VkExtent2D extent)
    {
        VkImageCreateInfo screenTextureInfo{};
        screenTextureInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        screenTextureInfo.imageType = VK_IMAGE_TYPE_2D;
        screenTextureInfo.format = format;
        screenTextureInfo.extent.width = extent.width;
        screenTextureInfo.extent.height = extent.height;
        screenTextureInfo.extent.depth = 1;
        screenTextureInfo.mipLevels = 1;
        screenTextureInfo.arrayLayers = 1;
        screenTextureInfo.tiling = VK_IMAGE_TILING_LINEAR;
        screenTextureInfo.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
        screenTextureInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        screenTextureInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        screenTextureInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        return screenTextureInfo;
    }

    void ComputeRTApp::createScreenTextureImageView()
    {
        VkImageViewCreateInfo screenTextureViewInfo{};
        screenTextureViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        screenTextureViewInfo.image = screenTextureImage.getImage();
        screenTextureViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        screenTextureViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        screenTextureViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        screenTextureViewInfo.subresourceRange.baseMipLevel = 0;
        screenTextureViewInfo.subresourceRange.levelCount = 1;
        screenTextureViewInfo.subresourceRange.baseArrayLayer = 0;
        screenTextureViewInfo.subresourceRange.layerCount = 1;

        screenTextureImage.createImageView(0, &screenTextureViewInfo);
    }

} // namespace lve
