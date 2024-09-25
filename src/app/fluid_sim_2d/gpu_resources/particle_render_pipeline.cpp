#include "particle_render_pipeline.hpp"

// libs
#include "include/glm.hpp"

namespace app::fluidsim2d
{
ParticleRenderPipeline::ParticleRenderPipeline(
    lve::FrameManager &frameManager,
    ParticleBuffers &particleBuffers
)
    : lveFrameManager{frameManager}, particleBuffers{particleBuffers}
{
    descriptorPool =
        lve::DescriptorPool::Builder(lveFrameManager.getDevice())
            .setMaxSets(lve::SwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, lve::SwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, lve::SwapChain::MAX_FRAMES_IN_FLIGHT * 2)
            .build();

    descriptorSets.resize(lve::SwapChain::MAX_FRAMES_IN_FLIGHT);

    descriptorSetLayout =
        lve::DescriptorSetLayout::Builder(lveFrameManager.getDevice())
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

    recreateScreenTextureImage(lveFrameManager.getWindow().getExtent());
    updateDescriptorSets(true);

    lve::GraphicPipelineConfigInfo screenTexturePipelineConfigInfo{};
    screenTexturePipelineConfigInfo.vertFilePath = "screen_texture_shader.vert.spv";
    screenTexturePipelineConfigInfo.fragFilePath = "screen_texture_shader.frag.spv";
    screenTexturePipelineConfigInfo.renderPass = lveFrameManager.getSwapChainRenderPass();

    VkPushConstantRange screenExtentPushRange = {
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::vec2)
    };

    screenTextureRenderPipeline = std::make_unique<lve::GraphicPipeline>(
        lveFrameManager.getDevice(),
        lve::GraphicPipelineLayoutConfigInfo{
            .descriptorSetLayouts = {descriptorSetLayout->getDescriptorSetLayout()},
            .pushConstantRanges = {screenExtentPushRange}
        },
        screenTexturePipelineConfigInfo
    );
}

void ParticleRenderPipeline::renderTexture(VkCommandBuffer cmdBuffer)
{
    lve::renderScreenTexture(
        cmdBuffer,
        &descriptorSets[lveFrameManager.getFrameIndex()],
        screenTextureRenderPipeline->getPipelineLayout(),
        screenTextureRenderPipeline.get(),
        lveFrameManager.getWindow().getExtent()
    );
}

void ParticleRenderPipeline::updateDescriptorSets(bool needMemoryAlloc)
{
    VkDescriptorImageInfo screenTextureDescriptorInfo = screenTextureImage.getDescriptorImageInfo(
        0, samplerManager.getSampler(lve::SamplerType::DEFAULT)
    );
    auto particleBufferInfo = particleBuffers.getParticleBuffer().descriptorInfo();
    auto neighborBufferInfo = particleBuffers.getNeighborBuffer().descriptorInfo();

    for (int i = 0; i < descriptorSets.size(); i++)
    {
        lve::DescriptorWriter writer{*descriptorSetLayout, *descriptorPool};
        writer
            .writeImage(0, &screenTextureDescriptorInfo) // combined image sampler
            .writeBuffer(1, &particleBufferInfo)         // storage buffer
            .writeBuffer(2, &neighborBufferInfo);        // storage buffer

        if (needMemoryAlloc)
        {
            writer.allocateDescriptorSet(descriptorSets[i]);
        }
        writer.overwrite(descriptorSets[i]);
    }
}

void ParticleRenderPipeline::recreateScreenTextureImage(VkExtent2D extent)
{
    screenTextureImage = lve::Image(
        lveFrameManager.getDevice(),
        createScreenTextureInfo(extent),
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    createScreenTextureImageView();
}

VkImageCreateInfo ParticleRenderPipeline::createScreenTextureInfo(VkExtent2D extent)
{
    VkImageCreateInfo screenTextureInfo{};
    screenTextureInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    screenTextureInfo.imageType = VK_IMAGE_TYPE_2D;
    screenTextureInfo.extent.width = extent.width;
    screenTextureInfo.extent.height = extent.height;
    screenTextureInfo.extent.depth = 1;
    screenTextureInfo.mipLevels = 1;
    screenTextureInfo.arrayLayers = 1;
    screenTextureInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    screenTextureInfo.tiling = VK_IMAGE_TILING_LINEAR;
    screenTextureInfo.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    screenTextureInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    screenTextureInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    screenTextureInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    return screenTextureInfo;
}

void ParticleRenderPipeline::createScreenTextureImageView()
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
} // namespace app::fluidsim2d