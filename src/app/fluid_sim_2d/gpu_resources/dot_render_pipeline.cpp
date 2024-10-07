#include "dot_render_pipeline.hpp"

namespace app::fluidsim
{
DotRenderPipeline::DotRenderPipeline(
    lve::FrameManager &frameManager, SPH &fluidParticleSys)
    : lveFrameManager{frameManager}, fluidParticleSys{fluidParticleSys}
{
    lve::GraphicPipelineConfigInfo dotPipelineConfigInfo;
    dotPipelineConfigInfo.vertFilePath = "dot_2d.vert.spv";
    dotPipelineConfigInfo.fragFilePath = "dot_2d.frag.spv";
    dotPipelineConfigInfo.renderPass = lveFrameManager.getSwapChainRenderPass();
    dotPipelineConfigInfo.vertexBindingDescriptions = lve::Model::Vertex::getBindingDescriptions();
    dotPipelineConfigInfo.vertexAttributeDescriptions =
        lve::Model::Vertex::getAttributeDescriptions();

    dotRenderPipeline = std::make_unique<lve::GraphicPipeline>(
        lveFrameManager.getDevice(), lve::GraphicPipelineLayoutConfigInfo{}, dotPipelineConfigInfo);
}

const glm::vec3 upColor = glm::vec3(0.996f, 0.267f, 0.412f);
const glm::vec3 downColor = glm::vec3(0.435f, 0.525f, 0.984f);
const glm::vec3 leftColor = glm::vec3(0.984f, 0.851f, 0.353f);
const glm::vec3 rightColor = glm::vec3(0.400f, 0.851f, 0.549f);
const float maxDisplayVelocityMag = 200.0f;
const float maxDisplayVelocityMagSqr = maxDisplayVelocityMag * maxDisplayVelocityMag;
const float medHighVelocityMagSqr = maxDisplayVelocityMagSqr / 4.0f * 3.0f;
const float medVelocityMagSqr = maxDisplayVelocityMagSqr / 2.0f;
const glm::vec3 black = glm::vec3(0.0f, 0.0f, 0.0f);
const glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);

glm::vec3 getParticleColorByVelocity(glm::vec2 v)
{
    glm::vec2 velocityDir = glm::normalize(v);

    if (glm::dot(v, v) > maxDisplayVelocityMagSqr)
        v = velocityDir * maxDisplayVelocityMag;

    // find the color based on the velocity magnitude and direction
    glm::vec3 xColor, yColor;
    glm::vec3 defaultColor = white * 0.01f;

    float xIntensity = glm::clamp(glm::abs(v.x) / maxDisplayVelocityMag, 0.0f, 1.0f);
    if (velocityDir.x > 0.0f)
        xColor = glm::mix(defaultColor, leftColor, xIntensity);
    else
        xColor = glm::mix(defaultColor, rightColor, xIntensity);

    float yIntensity = glm::clamp(glm::abs(v.y) / maxDisplayVelocityMag, 0.0f, 1.0f);
    if (velocityDir.y > 0.0f)
        yColor = glm::mix(defaultColor, downColor, yIntensity);
    else
        yColor = glm::mix(defaultColor, upColor, yIntensity);

    float intensitySum = xIntensity + yIntensity;
    return glm::clamp(
        xColor * xIntensity / intensitySum + yColor * yIntensity / intensitySum, 0.0f, 1.0f);
}

void DotRenderPipeline::render(VkCommandBuffer cmdBuffer)
{
    dotRenderPipeline->bind(cmdBuffer);

    const float dataScale = fluidParticleSys.getDataScale();
    const std::vector<glm::vec2> &positionData = fluidParticleSys.getPositionData();
    const std::vector<glm::vec2> &velocityData = fluidParticleSys.getVelocityData();
    const std::vector<int> &firstParticleNeighborIndex =
        fluidParticleSys.getFirstParticleNeighborIndex();

    const VkExtent2D extent = lveFrameManager.getWindow().getExtent();
    const float halfWindowWidth = static_cast<float>(extent.width) * 0.5f;
    const float halfWindowHeight = static_cast<float>(extent.height) * 0.5f;
    const float halfDotSize = 2.0f;
    const float halfDotSizeX = halfDotSize / halfWindowWidth;
    const float halfDotSizeY = halfDotSize / halfWindowHeight;

    // create a quad for each particle
    lve::Model::Builder builder;
    builder.vertices.reserve(positionData.size() * 4);
    builder.indices.reserve(positionData.size() * 6);
    unsigned int vertexCnt = 0;
    for (size_t i = 0; i < positionData.size(); i++, vertexCnt += 4)
    {
        glm::vec2 center =
            positionData[i] / glm::vec2(halfWindowWidth, halfWindowHeight) / dataScale -
            glm::vec2(1.0f);
        glm::vec2 topLeft = center - glm::vec2(halfDotSizeX, halfDotSizeY);
        glm::vec2 topRight = center + glm::vec2(halfDotSizeX, -halfDotSizeY);
        glm::vec2 bottomLeft = center + glm::vec2(-halfDotSizeX, halfDotSizeY);
        glm::vec2 bottomRight = center + glm::vec2(halfDotSizeX, halfDotSizeY);

        // color based on velocity
        glm::vec2 velocity = velocityData[i];
        glm::vec3 color = getParticleColorByVelocity(velocity / dataScale);

        lve::Model::Vertex v0{
            glm::vec3(topLeft, 0.0f), color, glm::vec3(0.0f), glm::vec2(0.0f, 0.0f)};
        lve::Model::Vertex v1{
            glm::vec3(topRight, 0.0f), color, glm::vec3(0.0f), glm::vec2(1.0f, 0.0f)};
        lve::Model::Vertex v2{
            glm::vec3(bottomLeft, 0.0f), color, glm::vec3(0.0f), glm::vec2(0.0f, 1.0f)};
        lve::Model::Vertex v3{
            glm::vec3(bottomRight, 0.0f), color, glm::vec3(0.0f), glm::vec2(1.0f, 1.0f)};

        builder.vertices.insert(builder.vertices.end(), {v0, v1, v2, v3});
        builder.indices.insert(
            builder.indices.end(),
            {vertexCnt, vertexCnt + 1, vertexCnt + 2, vertexCnt + 1, vertexCnt + 2, vertexCnt + 3});
    }

    // Apply the neighbor view
    if (fluidParticleSys.isNeighborViewOn())
    {
        // particle with index 0 should be red
        builder.vertices[0].color = glm::vec3(1.0f, 0.0f, 0.0f);
        builder.vertices[1].color = glm::vec3(1.0f, 0.0f, 0.0f);
        builder.vertices[2].color = glm::vec3(1.0f, 0.0f, 0.0f);
        builder.vertices[3].color = glm::vec3(1.0f, 0.0f, 0.0f);

        // particles with index in firstParticleNeighborIndex should be white
        for (int i : firstParticleNeighborIndex)
        {
            builder.vertices[i * 4].color = glm::vec3(1.0f, 1.0f, 1.0f);
            builder.vertices[i * 4 + 1].color = glm::vec3(1.0f, 1.0f, 1.0f);
            builder.vertices[i * 4 + 2].color = glm::vec3(1.0f, 1.0f, 1.0f);
            builder.vertices[i * 4 + 3].color = glm::vec3(1.0f, 1.0f, 1.0f);
        }
    }

    quads = std::make_unique<lve::Model>(lveFrameManager.getDevice(), builder);

    quads->bind(cmdBuffer);
    quads->draw(cmdBuffer);
}
} // namespace app::fluidsim