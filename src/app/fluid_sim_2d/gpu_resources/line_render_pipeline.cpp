#include "line_render_pipeline.hpp"

namespace app::fluidsim
{
LineRenderPipeline::LineRenderPipeline(
    lve::FrameManager &frameManager, SPH &fluidParticleSys)
    : lveFrameManager{frameManager}, fluidParticleSys{fluidParticleSys}
{
    lve::GraphicPipelineConfigInfo linePipelineConfigInfo;
    linePipelineConfigInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    linePipelineConfigInfo.vertFilePath = "line_2d.vert.spv";
    linePipelineConfigInfo.fragFilePath = "line_2d.frag.spv";
    linePipelineConfigInfo.renderPass = lveFrameManager.getSwapChainRenderPass();
    linePipelineConfigInfo.vertexBindingDescriptions = lve::Line::Vertex::getBindingDescriptions();
    linePipelineConfigInfo.vertexAttributeDescriptions =
        lve::Line::Vertex::getAttributeDescriptions();

    lineRenderPipeline = std::make_unique<lve::GraphicPipeline>(
        lveFrameManager.getDevice(),
        lve::GraphicPipelineLayoutConfigInfo{},
        linePipelineConfigInfo);
}

void LineRenderPipeline::render(VkCommandBuffer cmdBuffer)
{
    lineCollection.clearLines();
    lineCollection.addLines(fluidParticleSys.getDebugLines());
    lve::renderLines(cmdBuffer, lineRenderPipeline.get(), lineCollection);
}
} // namespace app::fluidsim