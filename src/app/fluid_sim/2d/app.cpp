#include "app/fluid_sim/2d/app.hpp"

#include "lve/core/resource/buffer.hpp"
#include "lve/core/resource/sampler_manager.hpp"
#include "lve/util/file_io.hpp"
#include "lve/util/math.hpp"

// libs
#include "include/glm.hpp"

// std
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <thread>

struct GlobalUbo
{
    glm::mat4 projectionView{1.f};
    glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f}; // w is intensity
    glm::vec3 lightPosition{-1.f};
    alignas(16) glm::vec4 lightColor{1.f}; // w is light intensity
};

FluidSim2DApp::FluidSim2DApp()
{
    // resize window according to config
    lve::io::YamlConfig config("config/fluidSim2D.yaml");
    std::vector<int> windowSize = config.get<std::vector<int>>("windowSize");
    lveWindow.resize(windowSize[0], windowSize[1]);

    // register callback functions for window resize
    lveRenderer.registerSwapChainResizedCallback(
        WINDOW_RESIZED_CALLBACK_NAME,
        [this](VkExtent2D extent) {
            recreateScreenTextureImage(extent);
            updateGlobalDescriptorSets();
            fluidParticleSys.updateWindowExtent(extent);
        }
    );

    globalPool =
        lve::DescriptorPool::Builder(lveDevice)
            .setMaxSets(lve::SwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, lve::SwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, lve::SwapChain::MAX_FRAMES_IN_FLIGHT
            )
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, lve::SwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                lve::SwapChain::MAX_FRAMES_IN_FLIGHT * 2
            ) // particle buffer, neighbor buffer
            .build();

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

    initParticleBuffer();
    writeParticleBuffer();

    globalSetLayout =
        lve::DescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(
                1,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                VK_SHADER_STAGE_FRAGMENT_BIT
            ) // Frag shader input texture
            .addBinding(
                2,
                VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                VK_SHADER_STAGE_COMPUTE_BIT
            ) // Compute shader output texture
            .addBinding(
                3,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                VK_SHADER_STAGE_FRAGMENT_BIT
            ) // Frag shader input particle buffer
            .addBinding(
                4,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                VK_SHADER_STAGE_FRAGMENT_BIT
            ) // Frag shader input neighbor buffer
            .build();

    recreateScreenTextureImage(lveWindow.getExtent());
    updateGlobalDescriptorSets(true);

    lve::GraphicPipelineConfigInfo screenTexturePipelineConfigInfo{};
    screenTexturePipelineConfigInfo.vertFilePath = "screen_texture_shader.vert.spv";
    screenTexturePipelineConfigInfo.fragFilePath = "screen_texture_shader.frag.spv";
    screenTexturePipelineConfigInfo.renderPass = lveRenderer.getSwapChainRenderPass();

    lve::GraphicPipelineConfigInfo linePipelineConfigInfo{};
    linePipelineConfigInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    linePipelineConfigInfo.vertFilePath = "line_2d.vert.spv";
    linePipelineConfigInfo.fragFilePath = "line_2d.frag.spv";
    linePipelineConfigInfo.renderPass = lveRenderer.getSwapChainRenderPass();
    linePipelineConfigInfo.vertexBindingDescriptions = lve::Line::Vertex::getBindingDescriptions();
    linePipelineConfigInfo.vertexAttributeDescriptions =
        lve::Line::Vertex::getAttributeDescriptions();

    VkPushConstantRange screenExtentPushRange = {
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(lve::ScreenExtentPushConstantData)
    };
    screenTextureRenderPipeline = lve::GraphicPipeline(
        lveDevice,
        lve::GraphicPipelineLayoutConfigInfo{
            .descriptorSetLayouts = {globalSetLayout->getDescriptorSetLayout()},
            .pushConstantRanges = {screenExtentPushRange}
        },
        screenTexturePipelineConfigInfo
    );

    lineRenderPipeline = lve::GraphicPipeline(
        lveDevice,
        lve::GraphicPipelineLayoutConfigInfo{
            .descriptorSetLayouts = {globalSetLayout->getDescriptorSetLayout()}
        },
        linePipelineConfigInfo
    );

    fluidSimComputePipeline = lve::ComputePipeline(
        lveDevice, {globalSetLayout->getDescriptorSetLayout()}, "my_compute_shader.comp.spv"
    );
}

void FluidSim2DApp::run()
{
    std::thread renderThread(&FluidSim2DApp::renderLoop, this);

    lveWindow.mainThreadGlfwEventLoop();

    isRunning = false;
    renderThread.join();

    vkDeviceWaitIdle(lveDevice.vkDevice());
}

void FluidSim2DApp::updateGlobalDescriptorSets(bool needMemoryAlloc)
{
    VkDescriptorImageInfo screenTextureDescriptorInfo = screenTextureImage.getDescriptorImageInfo(
        0, samplerManager.getSampler({lve::SamplerType::DEFAULT})
    );
    auto particleBufferInfo = particleBuffer->descriptorInfo();
    auto neighborBufferInfo = neighborBuffer->descriptorInfo();

    for (int i = 0; i < globalDescriptorSets.size(); i++)
    {
        auto uboBufferInfo = uboBuffers[i]->descriptorInfo();
        lve::DescriptorWriter writer{*globalSetLayout, *globalPool};
        writer.writeBuffer(0, &uboBufferInfo)
            .writeImage(1, &screenTextureDescriptorInfo) // combined image sampler
            .writeImage(2, &screenTextureDescriptorInfo) // storage image
            .writeBuffer(3, &particleBufferInfo)         // storage buffer
            .writeBuffer(4, &neighborBufferInfo);        // storage buffer

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
    screenTextureImage = lve::Image(
        lveDevice,
        createScreenTextureInfo(screenTextureFormat, extent),
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    createScreenTextureImageView();
}

void FluidSim2DApp::initParticleBuffer()
{
    int particleCount = fluidParticleSys.getParticleCount();

    particleBuffer = std::make_unique<lve::Buffer>(
        lveDevice,
        sizeof(int) +                           // particle count
            sizeof(float) +                     // smoothing radius
            sizeof(float) +                     // target density
            sizeof(float) +                     // data scale
            sizeof(uint32_t) +                  // isNeighborViewActive
            sizeof(uint32_t) +                  // isDensityViewActive
            sizeof(glm::vec2) * particleCount + // position
            sizeof(glm::vec2) * particleCount,  // velocity
        1,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    particleBuffer->map();
    particleBuffer->writeToBuffer(&particleCount, sizeof(int));

    neighborBuffer = std::make_unique<lve::Buffer>(
        lveDevice,
        sizeof(int) * particleCount,
        1,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    neighborBuffer->map();
}

void FluidSim2DApp::writeParticleBuffer()
{
    int particleCount = fluidParticleSys.getParticleCount();
    float smoothRadius = fluidParticleSys.getSmoothRadius();
    float targetDensity = fluidParticleSys.getTargetDensity();
    float dataScale = fluidParticleSys.getDataScale();
    uint32_t isNeighborViewActive = static_cast<uint32_t>(fluidParticleSys.isNeighborViewOn());
    uint32_t isDensityViewActive = static_cast<uint32_t>(fluidParticleSys.isDensityViewOn());

    particleBuffer->setRecordedOffset(sizeof(int));
    particleBuffer->writeToBufferOrdered(&smoothRadius, sizeof(float));
    particleBuffer->writeToBufferOrdered(&targetDensity, sizeof(float));
    particleBuffer->writeToBufferOrdered(&dataScale, sizeof(float));
    particleBuffer->writeToBufferOrdered(&isNeighborViewActive, sizeof(uint32_t));
    particleBuffer->writeToBufferOrdered(&isDensityViewActive, sizeof(uint32_t));
    particleBuffer->writeToBufferOrdered(
        (void *)fluidParticleSys.getPositionData().data(), sizeof(glm::vec2) * particleCount
    );
    particleBuffer->writeToBufferOrdered(
        (void *)fluidParticleSys.getVelocityData().data(), sizeof(glm::vec2) * particleCount
    );

    neighborBuffer->writeToBuffer((void *)fluidParticleSys.getFirstParticleNeighborIndex().data());
}

void FluidSim2DApp::drawDebugLines(VkCommandBuffer cmdBuffer)
{
    if (!fluidParticleSys.isDebugLineOn())
        return;

    lineCollection.clearLines();
    lineCollection.addLines(fluidParticleSys.getDebugLines());
    lve::renderLines(
        cmdBuffer,
        &globalDescriptorSets[lveRenderer.getFrameIndex()],
        lineRenderPipeline.getPipelineLayout(),
        &lineRenderPipeline,
        lineCollection
    );
}

void FluidSim2DApp::handleInput()
{
    if (lveWindow.input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) ||
        lveWindow.input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
    {
        double mouseX, mouseY;
        lveWindow.input.getMousePosition(mouseX, mouseY);
        glm::vec2 mousePos = {static_cast<float>(mouseX), static_cast<float>(mouseY)};
        fluidParticleSys.setRangeForcePos(
            lveWindow.input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT), mousePos
        );
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
    lveWindow.input.oneTimeKeyUse(GLFW_KEY_D, [this] {
        fluidParticleSys.toggleDensityView();
    });
}

void FluidSim2DApp::renderLoop()
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto now = currentTime;
    bool oneSecondPassed;
    while (isRunning)
    {
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime =
            std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime)
                .count();
        frameTime = std::min(frameTime, maxFrameTime);
        currentTime = newTime;

        fpsCounter.frameCount++;
        if (std::chrono::duration<float, std::chrono::seconds::period>(
                currentTime - fpsCounter.startTime
            )
                .count() >= 1.0f)
        {
            lveWindow.setTitle(APP_NAME + " (FPS: " + std::to_string(fpsCounter.frameCount) + ")");
            fpsCounter.frameCount = 0;
            fpsCounter.startTime = currentTime;
        }
        oneSecondPassed = fpsCounter.frameCount == 0;

        if (auto commandBuffer = lveRenderer.beginFrame())
        {
            int frameIndex = lveRenderer.getFrameIndex();

            // update
            windowExtent = lveWindow.getExtent();
            fluidSimComputePipeline.dispatchComputePipeline(
                commandBuffer,
                &globalDescriptorSets[frameIndex],
                static_cast<int>(std::ceil(windowExtent.width / 8.f)),
                static_cast<int>(std::ceil(windowExtent.height / 8.f))
            );

            handleInput();

            // fluid particle system
            fluidParticleSys.updateParticleData(frameTime);
            writeParticleBuffer();

            // render
            lveRenderer.beginSwapChainRenderPass(commandBuffer);

            lve::renderScreenTexture(
                commandBuffer,
                &globalDescriptorSets[frameIndex],
                screenTextureRenderPipeline.getPipelineLayout(),
                &screenTextureRenderPipeline,
                windowExtent
            );

            drawDebugLines(commandBuffer);

            lveRenderer.endSwapChainRenderPass(commandBuffer);
            lveRenderer.endFrame();
        }
    }
}