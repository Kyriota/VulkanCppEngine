#include "particle_buffer.hpp"

namespace app::fluidsim2d
{
ParticleBuffers::ParticleBuffers(lve::FrameManager &frameManager, FluidParticleSystem &fluidParticleSys)
    : lveFrameManager(frameManager), fluidParticleSys(fluidParticleSys)
{
    initParticleBuffer();
    writeParticleBuffer();
}

void ParticleBuffers::initParticleBuffer()
{
    int particleCount = fluidParticleSys.getParticleCount();
    particleBuffer = std::make_unique<lve::Buffer>(
        lveFrameManager.getDevice(),
        sizeof(int) +                           // particle count
            sizeof(float) +                     // smoothing radius
            sizeof(float) +                     // target density
            sizeof(float) +                     // data scale
            sizeof(uint32_t) +                  // isNeighborViewActive
            sizeof(uint32_t) +                  // isDensityViewActive
            sizeof(glm::vec2) * particleCount + // position
            sizeof(glm::vec2) * particleCount,  // velocity
        1,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    particleBuffer->map();
    particleBuffer->writeToBuffer(&particleCount, sizeof(int));

    neighborBuffer = std::make_unique<lve::Buffer>(
        lveFrameManager.getDevice(),
        sizeof(int) * particleCount,
        1,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    neighborBuffer->map();
}

void ParticleBuffers::writeParticleBuffer()
{
    int particleCount = fluidParticleSys.getParticleCount();
    float smoothRadius = fluidParticleSys.getSmoothRadius();
    float targetDensity = fluidParticleSys.getTargetDensity();
    float dataScale = fluidParticleSys.getDataScale();
    uint32_t isNeighborViewActive = static_cast<uint32_t>(fluidParticleSys.isNeighborViewOn());
    uint32_t isDensityViewActive = static_cast<uint32_t>(fluidParticleSys.isDensityViewOn());

    particleBuffer->setRecordedOffset(sizeof(int));
    particleBuffer->writeToBufferOrdered(&smoothRadius, sizeof(float));
    particleBuffer->writeToBufferOrdered(&targetDensity, sizeof(float));
    particleBuffer->writeToBufferOrdered(&dataScale, sizeof(float));
    particleBuffer->writeToBufferOrdered(&isNeighborViewActive, sizeof(uint32_t));
    particleBuffer->writeToBufferOrdered(&isDensityViewActive, sizeof(uint32_t));
    particleBuffer->writeToBufferOrdered(
        (void *)fluidParticleSys.getPositionData().data(), sizeof(glm::vec2) * particleCount
    );
    particleBuffer->writeToBufferOrdered(
        (void *)fluidParticleSys.getVelocityData().data(), sizeof(glm::vec2) * particleCount
    );

    neighborBuffer->writeToBuffer((void *)fluidParticleSys.getFirstParticleNeighborIndex().data());
}
} // namespace app::fluidsim2d