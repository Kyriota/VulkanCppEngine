#include "app/fluid_sim_2d/gpu_resources.hpp"

#include "lve/core/swap_chain.hpp"

// libs
#include "include/glm.hpp"

namespace app::fluidsim2d
{
struct GlobalUbo
{
    glm::mat4 projectionView{1.f};
    glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f}; // w is intensity
    glm::vec3 lightPosition{-1.f};
    alignas(16) glm::vec4 lightColor{1.f}; // w is light intensity
};

GpuResources::GpuResources(lve::FrameManager &frameManager, FluidParticleSystem &fluidParticleSys)
    : lveFrameManager{frameManager}, fluidParticleSys{fluidParticleSys}
{
    globalPool =
        lve::DescriptorPool::Builder(lveFrameManager.getDevice())
            .setMaxSets(lve::SwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, lve::SwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, lve::SwapChain::MAX_FRAMES_IN_FLIGHT)
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
            lveFrameManager.getDevice(),
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
        lve::DescriptorSetLayout::Builder(lveFrameManager.getDevice())
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

    recreateScreenTextureImage(lveFrameManager.getWindow().getExtent());
    updateGlobalDescriptorSets(true);

    lve::GraphicPipelineConfigInfo screenTexturePipelineConfigInfo{};
    screenTexturePipelineConfigInfo.vertFilePath = "screen_texture_shader.vert.spv";
    screenTexturePipelineConfigInfo.fragFilePath = "screen_texture_shader.frag.spv";
    screenTexturePipelineConfigInfo.renderPass = lveFrameManager.getSwapChainRenderPass();

    lve::GraphicPipelineConfigInfo linePipelineConfigInfo{};
    linePipelineConfigInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    linePipelineConfigInfo.vertFilePath = "line_2d.vert.spv";
    linePipelineConfigInfo.fragFilePath = "line_2d.frag.spv";
    linePipelineConfigInfo.renderPass = lveFrameManager.getSwapChainRenderPass();
    linePipelineConfigInfo.vertexBindingDescriptions = lve::Line::Vertex::getBindingDescriptions();
    linePipelineConfigInfo.vertexAttributeDescriptions =
        lve::Line::Vertex::getAttributeDescriptions();

    VkPushConstantRange screenExtentPushRange = {
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(lve::ScreenExtentPushConstantData)
    };
    screenTextureRenderPipeline = std::make_unique<lve::GraphicPipeline>(
        lveFrameManager.getDevice(),
        lve::GraphicPipelineLayoutConfigInfo{
            .descriptorSetLayouts = {globalSetLayout->getDescriptorSetLayout()},
            .pushConstantRanges = {screenExtentPushRange}
        },
        screenTexturePipelineConfigInfo
    );

    lineRenderPipeline = std::make_unique<lve::GraphicPipeline>(
        lveFrameManager.getDevice(),
        lve::GraphicPipelineLayoutConfigInfo{
            .descriptorSetLayouts = {globalSetLayout->getDescriptorSetLayout()}
        },
        linePipelineConfigInfo
    );
}

void GpuResources::initParticleBuffer()
{
    int particleCount = fluidParticleSys.getParticleCount();
    particleBuffer = std::make_unique<lve::Buffer>(
        lveFrameManager.getDevice(),
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
        lveFrameManager.getDevice(),
        sizeof(int) * particleCount,
        1,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    neighborBuffer->map();
}

void GpuResources::writeParticleBuffer()
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

void GpuResources::updateGlobalDescriptorSets(bool needMemoryAlloc)
{
    VkDescriptorImageInfo screenTextureDescriptorInfo = screenTextureImage.getDescriptorImageInfo(
        0, samplerManager.getSampler(lve::SamplerType::DEFAULT)
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

VkImageCreateInfo GpuResources::createScreenTextureInfo(VkFormat format, VkExtent2D extent)
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

void GpuResources::createScreenTextureImageView()
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

void GpuResources::recreateScreenTextureImage(VkExtent2D extent)
{
    screenTextureImage = lve::Image(
        lveFrameManager.getDevice(),
        createScreenTextureInfo(VK_FORMAT_R8G8B8A8_UNORM, extent),
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    createScreenTextureImageView();
}

void GpuResources::drawDebugLines(VkCommandBuffer cmdBuffer)
{
    if (!fluidParticleSys.isDebugLineOn())
        return;

    lineCollection.clearLines();
    lineCollection.addLines(fluidParticleSys.getDebugLines());
    lve::renderLines(
        cmdBuffer,
        &globalDescriptorSets[lveFrameManager.getFrameIndex()],
        lineRenderPipeline->getPipelineLayout(),
        lineRenderPipeline.get(),
        lineCollection
    );
}

void GpuResources::renderTexture(VkCommandBuffer cmdBuffer)
{
    lve::renderScreenTexture(
        cmdBuffer,
        &globalDescriptorSets[lveFrameManager.getFrameIndex()],
        screenTextureRenderPipeline->getPipelineLayout(),
        screenTextureRenderPipeline.get(),
        lveFrameManager.getWindow().getExtent()
    );
}
} // namespace app::fluidsim2d