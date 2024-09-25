#pragma once

#include "lve/core/frame_manager.hpp"
#include "lve/core/pipeline/graphics_pipeline.hpp"
#include "lve/core/resource/buffer.hpp"
#include "lve/core/resource/descriptors.hpp"
#include "lve/core/resource/sampler_manager.hpp"
#include "lve/core/swap_chain.hpp"
#include "particle_buffer.hpp"

namespace app::fluidsim2d
{
class ParticleRenderPipeline
{
public: // constructors
    ParticleRenderPipeline(lve::FrameManager &frameManager, ParticleBuffers &particleBuffers);
    ParticleRenderPipeline(const ParticleRenderPipeline &) = delete;
    ParticleRenderPipeline &operator=(const ParticleRenderPipeline &) = delete;

public: // methods
    void renderTexture(VkCommandBuffer cmdBuffer);
    void updateDescriptorSets(bool needMemoryAlloc = false);
    void recreateScreenTextureImage(VkExtent2D extent);

private: // methods
    VkImageCreateInfo createScreenTextureInfo(VkExtent2D extent);
    void createScreenTextureImageView();

private: // variables
    lve::FrameManager &lveFrameManager;
    ParticleBuffers &particleBuffers;

    // resources
    lve::Image screenTextureImage{lveFrameManager.getDevice()};
    lve::SamplerManager samplerManager{lveFrameManager.getDevice()};

    // descriptors
    std::unique_ptr<lve::DescriptorPool> descriptorPool{};
    std::unique_ptr<lve::DescriptorSetLayout> descriptorSetLayout;
    std::vector<VkDescriptorSet> descriptorSets;

    std::unique_ptr<lve::GraphicPipeline> screenTextureRenderPipeline;
};
} // namespace app::fluidsim2d