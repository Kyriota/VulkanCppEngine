#include "app/fluid_sim_2d/fluid_particle_system.hpp"
#include "lve/core/frame_manager.hpp"
#include "lve/core/pipeline/graphics_pipeline.hpp"
#include "lve/core/resource/buffer.hpp"
#include "lve/core/resource/descriptors.hpp"
#include "lve/core/resource/image.hpp"
#include "lve/core/resource/sampler_manager.hpp"

namespace app::fluidsim2d
{
class GpuResources
{
public: // constructors
    GpuResources(lve::FrameManager &frameManager, FluidParticleSystem &fluidParticleSys);
    GpuResources(const GpuResources &) = delete;
    GpuResources &operator=(const GpuResources &) = delete;

public: // methods
    void writeParticleBuffer();
    void renderTexture(VkCommandBuffer cmdBuffer);
    void drawDebugLines(VkCommandBuffer cmdBuffer);
    void updateGlobalDescriptorSets(bool needMemoryAlloc = false);
    void recreateScreenTextureImage(VkExtent2D extent);

    VkImageCreateInfo createScreenTextureInfo(VkFormat format, VkExtent2D extent);
    void createScreenTextureImageView();

private: // methods
    void initParticleBuffer();

private: // variables
    lve::FrameManager &lveFrameManager;
    FluidParticleSystem &fluidParticleSys;

    // resources
    lve::LineCollection lineCollection{lveFrameManager.getDevice(), fluidParticleSys.getParticleCount()};
    lve::Image screenTextureImage{lveFrameManager.getDevice()};
    lve::SamplerManager samplerManager{lveFrameManager.getDevice()};

    // buffers
    std::unique_ptr<lve::Buffer> particleBuffer;
    std::unique_ptr<lve::Buffer> neighborBuffer;
    std::vector<std::unique_ptr<lve::Buffer>> uboBuffers;

    // descriptors
    std::unique_ptr<lve::DescriptorPool> globalPool{};
    std::unique_ptr<lve::DescriptorSetLayout> globalSetLayout;
    std::vector<VkDescriptorSet> globalDescriptorSets;

    // pipelines
    std::unique_ptr<lve::GraphicPipeline> screenTextureRenderPipeline;
    std::unique_ptr<lve::GraphicPipeline> lineRenderPipeline;
};
} // namespace app::fluidsim2d