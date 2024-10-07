#pragma once

#include "../sph.hpp"

// lve
#include "lve/core/frame_manager.hpp"
#include "lve/core/pipeline/graphics_pipeline.hpp"
#include "lve/core/resource/descriptors.hpp"

namespace app::fluidsim
{
class LineRenderPipeline
{
public: // constructors
    LineRenderPipeline(lve::FrameManager &frameManager, SPH &fluidParticleSys);
    LineRenderPipeline(const LineRenderPipeline &) = delete;
    LineRenderPipeline &operator=(const LineRenderPipeline &) = delete;

public: // methods
    void render(VkCommandBuffer cmdBuffer);

private: // variables
    lve::FrameManager &lveFrameManager;
    SPH &fluidParticleSys;

    // resources
    lve::LineCollection lineCollection{
        lveFrameManager.getDevice(), fluidParticleSys.getParticleCount()};

    std::unique_ptr<lve::GraphicPipeline> lineRenderPipeline;
};
} // namespace app::fluidsim