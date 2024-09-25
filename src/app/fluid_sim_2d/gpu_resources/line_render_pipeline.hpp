#pragma once

#include "../fluid_particle_system.hpp"

// lve
#include "lve/core/frame_manager.hpp"
#include "lve/core/pipeline/graphics_pipeline.hpp"
#include "lve/core/resource/descriptors.hpp"

namespace app::fluidsim2d
{
class LineRenderPipeline
{
public: // constructors
    LineRenderPipeline(lve::FrameManager &frameManager, FluidParticleSystem &fluidParticleSys);
    LineRenderPipeline(const LineRenderPipeline &) = delete;
    LineRenderPipeline &operator=(const LineRenderPipeline &) = delete;

public: // methods
    void drawDebugLines(VkCommandBuffer cmdBuffer);

private: // variables
    lve::FrameManager &lveFrameManager;
    FluidParticleSystem &fluidParticleSys;

    // resources
    lve::LineCollection lineCollection{lveFrameManager.getDevice(), fluidParticleSys.getParticleCount()};

    std::unique_ptr<lve::GraphicPipeline> lineRenderPipeline;
};
} // namespace app::fluidsim2d