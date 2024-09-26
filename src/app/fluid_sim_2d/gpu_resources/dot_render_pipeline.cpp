#include "dot_render_pipeline.hpp"

namespace app::fluidsim2d
{
DotRenderPipeline::DotRenderPipeline(
    lve::FrameManager &frameManager,
    FluidParticleSystem &fluidParticleSys)
    : lveFrameManager{frameManager},
      fluidParticleSys{fluidParticleSys}
{
    lve::GraphicPipelineConfigInfo dotPipelineConfigInfo;
    dotPipelineConfigInfo.vertFilePath = "dot_2d.vert.spv";
    dotPipelineConfigInfo.fragFilePath = "dot_2d.frag.spv";
    dotPipelineConfigInfo.renderPass = lveFrameManager.getSwapChainRenderPass();
    dotPipelineConfigInfo.vertexBindingDescriptions = lve::Model::Vertex::getBindingDescriptions();
    dotPipelineConfigInfo.vertexAttributeDescriptions = lve::Model::Vertex::getAttributeDescriptions();

    dotRenderPipeline = std::make_unique<lve::GraphicPipeline>(
        lveFrameManager.getDevice(),
        lve::GraphicPipelineLayoutConfigInfo{},
        dotPipelineConfigInfo);
}

void DotRenderPipeline::render(VkCommandBuffer cmdBuffer)
{
    dotRenderPipeline->bind(cmdBuffer);

    const std::vector<glm::vec2> &positionData = fluidParticleSys.getPositionData();
    VkExtent2D extent = lveFrameManager.getWindow().getExtent();
    float halfWindowWidth = static_cast<float>(extent.width) * 0.5f;
    float halfWindowHeight = static_cast<float>(extent.height) * 0.5f;
    float halfDotSize = 5.0f;
    float halfDotSizeX = halfDotSize / halfWindowWidth;
    float halfDotSizeY = halfDotSize / halfWindowHeight;
    // create a quad for each particle
    lve::Model::Builder builder;
    unsigned int vertexCnt = 0;
    for (size_t i = 0; i < positionData.size(); i++, vertexCnt += 4)
    {
        glm::vec2 center = positionData[i] / glm::vec2(halfWindowWidth, halfWindowHeight) / fluidParticleSys.getDataScale() - glm::vec2(1.0f);
        glm::vec2 topLeft = center - glm::vec2(halfDotSizeX, halfDotSizeY);
        glm::vec2 topRight = center + glm::vec2(halfDotSizeX, -halfDotSizeY);
        glm::vec2 bottomLeft = center + glm::vec2(-halfDotSizeX, halfDotSizeY);
        glm::vec2 bottomRight = center + glm::vec2(halfDotSizeX, halfDotSizeY);

        lve::Model::Vertex v0{glm::vec3(topLeft, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f), glm::vec2(0.0f, 0.0f)};
        lve::Model::Vertex v1{glm::vec3(topRight, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 0.0f)};
        lve::Model::Vertex v2{glm::vec3(bottomLeft, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f), glm::vec2(0.0f, 1.0f)};
        lve::Model::Vertex v3{glm::vec3(bottomRight, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f), glm::vec2(1.0f, 1.0f)};

        builder.vertices.insert(builder.vertices.end(), {v0, v1, v2, v3});
        builder.indices.insert(builder.indices.end(), {vertexCnt, vertexCnt + 1, vertexCnt + 2, vertexCnt + 1, vertexCnt + 2, vertexCnt + 3});
    }

    quads = std::make_unique<lve::Model>(lveFrameManager.getDevice(), builder);

    quads->bind(cmdBuffer);
    quads->draw(cmdBuffer);
}
} // namespace app::fluidsim2d