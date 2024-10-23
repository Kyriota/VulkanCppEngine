#include "dot_render_pipeline.hpp"

namespace app::fluidsim
{
DotRenderPipeline::DotRenderPipeline(lve::FrameManager &frameManager, MPM &fluidParticleSys)
    : lveFrameManager{frameManager}, fluidParticleSys{fluidParticleSys}
{
    lve::GraphicPipelineConfigInfo dotPipelineConfigInfo;
    dotPipelineConfigInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    dotPipelineConfigInfo.vertFilePath = "dot_2d.vert.spv";
    dotPipelineConfigInfo.fragFilePath = "dot_2d.frag.spv";
    dotPipelineConfigInfo.renderPass = lveFrameManager.getSwapChainRenderPass();
    dotPipelineConfigInfo.vertexBindingDescriptions = lve::Point::getBindingDescriptions();
    dotPipelineConfigInfo.vertexAttributeDescriptions = lve::Point::getAttributeDescriptions();

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
const glm::vec3 darkGray = glm::vec3(0.1f, 0.1f, 0.1f);
const glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);

glm::vec3 getParticleColorByVelocity(glm::vec2 v)
{
    float velMagSqr = glm::dot(v, v);
    if (velMagSqr < 0.00001f)
        return darkGray;

    glm::vec2 velocityDir = glm::normalize(v);
    if (velMagSqr > maxDisplayVelocityMagSqr)
        v = velocityDir * maxDisplayVelocityMag;

    // find the color based on the velocity magnitude and direction
    glm::vec3 xColor, yColor;

    float xIntensity = glm::clamp(glm::abs(v.x) / maxDisplayVelocityMag, 0.0f, 1.0f);
    if (velocityDir.x > 0.0f)
        xColor = glm::mix(darkGray, leftColor, xIntensity);
    else
        xColor = glm::mix(darkGray, rightColor, xIntensity);

    float yIntensity = glm::clamp(glm::abs(v.y) / maxDisplayVelocityMag, 0.0f, 1.0f);
    if (velocityDir.y > 0.0f)
        yColor = glm::mix(darkGray, downColor, yIntensity);
    else
        yColor = glm::mix(darkGray, upColor, yIntensity);

    float intensitySum = xIntensity + yIntensity;
    return glm::clamp(
        xColor * xIntensity / intensitySum + yColor * yIntensity / intensitySum, 0.0f, 1.0f);
}

void DotRenderPipeline::render(VkCommandBuffer cmdBuffer)
{
    pointCollection.clearPoints();
    dotRenderPipeline->bind(cmdBuffer);

    const float dataScale = 1.0f / 400.0f;
    const std::vector<glm::vec2> &positionData = fluidParticleSys.getPositionData();
    const std::vector<glm::vec2> &velocityData = fluidParticleSys.getVelocityData();
    // const std::vector<int> &firstParticleNeighborIndex =
    //     fluidParticleSys.getFirstParticleNeighborIndex();

    const VkExtent2D extent = lveFrameManager.getWindow().getExtent();
    const float halfWindowWidth = static_cast<float>(extent.width) * 0.5f;
    const float halfWindowHeight = static_cast<float>(extent.height) * 0.5f;

    // Add particles to the point collection
    std::vector<lve::Point> points;
    points.reserve(positionData.size());
    for (size_t i = 0; i < positionData.size(); i++)
    {
        glm::vec2 center =
            positionData[i] / glm::vec2(halfWindowWidth, halfWindowHeight) / dataScale -
            glm::vec2(1.0f);
        glm::vec2 velocity = velocityData[i];
        glm::vec3 color = getParticleColorByVelocity(velocity / dataScale);

        points.push_back(lve::Point{glm::vec3(center, 0.0f), glm::vec4(color, 1.0f), 1.0f});
    }
    pointCollection.addPoints(points);
    
    pointCollection.bind(cmdBuffer);
    pointCollection.draw(cmdBuffer);
}
} // namespace app::fluidsim