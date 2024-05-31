#include "app.hpp"

#include "lve/lve_buffer.hpp"
#include "lve/lve_sampler_manager.hpp"

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
#include <thread>
#include <iostream>

namespace lve
{

    struct GlobalUbo
    {
        glm::mat4 projectionView{1.f};
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f}; // w is intensity
        glm::vec3 lightPosition{-1.f};
        alignas(16) glm::vec4 lightColor{1.f}; // w is light intensity
    };

    const std::string FluidSim2DApp::WINDOW_RESIZED_CALLBACK_NAME = "FluidSim2DApp";

    FluidSim2DApp::FluidSim2DApp()
    {
        globalPool =
            LveDescriptorPool::Builder(lveDevice)
                .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                .build();

        // register callback functions for window resize
        lveRenderer.registerSwapChainResizedCallback(
            WINDOW_RESIZED_CALLBACK_NAME,
            [this](VkExtent2D extent)
            {
                recreateScreenTextureImage(extent);
                updateGlobalDescriptorSets();
            });

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

        initParticleBuffer(fluidParticleSys.getParticleData());
        writeParticleBuffer(fluidParticleSys.getParticleData());

        globalSetLayout =
            LveDescriptorSetLayout::Builder(lveDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Frag shader input texture
                .addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)           // Compute shader output texture
                .addBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)         // Frag shader input particle buffer
                .build();

        recreateScreenTextureImage(lveWindow.getExtent());
        updateGlobalDescriptorSets(true);

        GraphicPipelineConfigInfo screenTexturePipelineConfigInfo{};
        screenTexturePipelineConfigInfo.vertFilepath = "build/shaders/screen_texture_shader.vert.spv";
        screenTexturePipelineConfigInfo.fragFilepath = "build/shaders/screen_texture_shader.frag.spv";

        screenTextureRenderSystem = RenderSystem(
            lveDevice,
            lveRenderer.getSwapChainRenderPass(),
            {globalSetLayout->getDescriptorSetLayout()},
            screenTexturePipelineConfigInfo);

        fluidSimComputeSystem = ComputeSystem(
            lveDevice,
            {globalSetLayout->getDescriptorSetLayout()},
            "build/shaders/my_compute_shader.comp.spv");
    }

    FluidSim2DApp::~FluidSim2DApp()
    {
        LveSamplerManager::clearSamplers();
    }

    void FluidSim2DApp::run()
    {
        std::thread renderThread(&FluidSim2DApp::renderLoop, this);

        lveWindow.mainThreadGlfwEventLoop();

        isRunning = false;
        renderThread.join();

        vkDeviceWaitIdle(lveDevice.device());
    }

    void FluidSim2DApp::updateGlobalDescriptorSets(bool needMemoryAlloc)
    {
        VkDescriptorImageInfo screenTextureDescriptorInfo = screenTextureImage.getDescriptorImageInfo(
            0, LveSamplerManager::getSampler({SamplerType::DEFAULT, lveDevice.device()}));
        auto particleBufferInfo = particleBuffer->descriptorInfo();

        for (int i = 0; i < globalDescriptorSets.size(); i++)
        {
            auto uboBufferInfo = uboBuffers[i]->descriptorInfo();
            LveDescriptorWriter writer{*globalSetLayout, *globalPool};
            writer.writeBuffer(0, &uboBufferInfo)
                .writeImage(1, &screenTextureDescriptorInfo) // combined image sampler
                .writeImage(2, &screenTextureDescriptorInfo) // storage image
                .writeBuffer(3, &particleBufferInfo);        // storage buffer

            if (needMemoryAlloc)
            {
                writer.allocateDescriptorSet(globalDescriptorSets[i]);
            }
            writer.overwrite(globalDescriptorSets[i]);
        }
    }

    VkImageCreateInfo FluidSim2DApp::createScreenTextureInfo(VkFormat format, VkExtent2D extent)
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

    void FluidSim2DApp::createScreenTextureImageView()
    {
        VkImageViewCreateInfo screenTextureViewInfo{};
        screenTextureViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        screenTextureViewInfo.image = screenTextureImage.getImage();
        screenTextureViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        screenTextureViewInfo.format = screenTextureFormat;
        screenTextureViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        screenTextureViewInfo.subresourceRange.baseMipLevel = 0;
        screenTextureViewInfo.subresourceRange.levelCount = 1;
        screenTextureViewInfo.subresourceRange.baseArrayLayer = 0;
        screenTextureViewInfo.subresourceRange.layerCount = 1;

        screenTextureImage.createImageView(0, &screenTextureViewInfo);
    }

    void FluidSim2DApp::recreateScreenTextureImage(VkExtent2D extent)
    {
        screenTextureImage = LveImage(
            lveDevice,
            createScreenTextureInfo(screenTextureFormat, extent),
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        createScreenTextureImageView();
    }

    void FluidSim2DApp::initParticleBuffer(FluidParticleSystem::ParticleData &particleData)
    {
        int particleCount = particleData.numParticles;
        float smoothingRadius = particleData.smoothingRadius;

        particleBuffer = std::make_unique<LveBuffer>(
            lveDevice,
            sizeof(int) +                           // particle count
                sizeof(float) +                     // smoothing radius
                sizeof(glm::vec2) * particleCount + // positions
                sizeof(glm::vec2) * particleCount,  // velocities
            1,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        particleBuffer->map();
        particleBuffer->setRecordedOffset(0);
        particleBuffer->writeToBufferOrdered(&particleCount, sizeof(int));
        particleBuffer->writeToBufferOrdered(&smoothingRadius, sizeof(float));
        // print smoothing radius and size of float
        std::cout << "smoothing radius: " << smoothingRadius << std::endl;
        std::cout << "size of float: " << sizeof(float) << std::endl;
    }

    void FluidSim2DApp::writeParticleBuffer(FluidParticleSystem::ParticleData &particleData)
    {
        particleBuffer->setRecordedOffset(sizeof(int) + sizeof(float));
        particleBuffer->writeToBufferOrdered(particleData.positions.data(), sizeof(glm::vec2) * particleData.numParticles);
        particleBuffer->writeToBufferOrdered(particleData.velocities.data(), sizeof(glm::vec2) * particleData.numParticles);
    }

    void FluidSim2DApp::renderLoop()
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        while (isRunning)
        {
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            if (auto commandBuffer = lveRenderer.beginFrame())
            {
                int frameIndex = lveRenderer.getFrameIndex();

                // update
                windowExtent = lveWindow.getExtent();
                fluidSimComputeSystem.dispatchComputePipeline(
                    commandBuffer,
                    &globalDescriptorSets[frameIndex],
                    static_cast<int>(std::ceil(windowExtent.width / 8.f)),
                    static_cast<int>(std::ceil(windowExtent.height / 8.f)));

                fluidParticleSys.updateWindowExtent(windowExtent);
                fluidParticleSys.updateParticleData(frameTime);
                writeParticleBuffer(fluidParticleSys.getParticleData());

                // render
                lveRenderer.beginSwapChainRenderPass(commandBuffer);

                renderScreenTexture(
                    commandBuffer,
                    &globalDescriptorSets[frameIndex],
                    screenTextureRenderSystem.getPipelineLayout(),
                    screenTextureRenderSystem.getPipeline(),
                    windowExtent);

                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
        }
    }
} // namespace lve
