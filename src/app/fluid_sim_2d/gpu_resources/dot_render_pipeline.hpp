#pragma once

#include "../mpm.hpp"

// lve
#include "lve/GO/geo/point.hpp"
#include "lve/core/frame_manager.hpp"
#include "lve/core/pipeline/graphics_pipeline.hpp"
#include "lve/core/resource/buffer.hpp"
#include "lve/core/resource/descriptors.hpp"
#include "lve/core/resource/sampler_manager.hpp"
#include "lve/core/swap_chain.hpp"

namespace app::fluidsim
{
class DotRenderPipeline
{
public: // constructors
    DotRenderPipeline(
        lve::FrameManager &frameManager,
        MPM &fluidParticleSys);
    DotRenderPipeline(const DotRenderPipeline &) = delete;
    DotRenderPipeline &operator=(const DotRenderPipeline &) = delete;

public: // methods
    void render(VkCommandBuffer cmdBuffer);

private: // variables
    lve::FrameManager &lveFrameManager;
    MPM &fluidParticleSys;

    // resources
    lve::PointCollection pointCollection{
        lveFrameManager.getDevice(), fluidParticleSys.getParticleCount()};

    std::unique_ptr<lve::GraphicPipeline> dotRenderPipeline;
};
} // namespace app::fluidsim