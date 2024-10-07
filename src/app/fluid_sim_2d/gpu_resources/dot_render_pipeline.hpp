#pragma once

#include "../sph.hpp"

#include "lve/GO/geo/model.hpp"
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
        SPH &fluidParticleSys);
    DotRenderPipeline(const DotRenderPipeline &) = delete;
    DotRenderPipeline &operator=(const DotRenderPipeline &) = delete;

public: // methods
    void render(VkCommandBuffer cmdBuffer);

private: // variables
    lve::FrameManager &lveFrameManager;
    SPH &fluidParticleSys;

    // resources
    std::unique_ptr<lve::Model> quads;

    std::unique_ptr<lve::GraphicPipeline> dotRenderPipeline;
};
} // namespace app::fluidsim