#include "fluid_particle_system.hpp"

// std
#include <algorithm>

namespace lve
{
    FluidParticleSystem::ParticleData FluidParticleSystem::initParticleData(glm::vec2 startPoint, float stride, float maxWidth)
    {
        ParticleData particleBufferData{};
        particleBufferData.numParticles = particleCount;
        particleBufferData.positions.resize(particleCount);
        particleBufferData.velocities.resize(particleCount);

        maxWidth -= std::fmod(maxWidth, stride);
        int cntPerRow = static_cast<int>(maxWidth / stride);
        int row, col;
        for (int i = 0; i < particleCount; i++)
        {
            row = static_cast<int>(i / cntPerRow);
            col = i % cntPerRow;
            particleBufferData.positions[i] = startPoint + glm::vec2(col * stride, row * stride);

            // random velocity ranged from -1 to 1
            particleBufferData.velocities[i] = glm::vec2(
                static_cast<float>(rand() % 400) - 200.f,
                static_cast<float>(rand() % 400) - 200.f);
        }

        return particleBufferData;
    }

    void FluidParticleSystem::updateParticleData(float deltaTime)
    {
        // particle update rule
        for (int i = 0; i < particleCount; i++)
        {
            particleData.positions[i] += particleData.velocities[i] * deltaTime;
        }

        // boundary collision
        for (int i = 0; i < particleCount; i++)
        {
            if (particleData.positions[i].x < 0 || particleData.positions[i].x > windowExtent.width)
            {
                particleData.positions[i].x = std::clamp(particleData.positions[i].x, 0.f, static_cast<float>(windowExtent.width));
                particleData.velocities[i].x *= -collisionDamping;
            }
            if (particleData.positions[i].y < 0 || particleData.positions[i].y > windowExtent.height)
            {
                particleData.positions[i].y = std::clamp(particleData.positions[i].y, 0.f, static_cast<float>(windowExtent.height));
                particleData.velocities[i].y *= -collisionDamping;
            }
        }
    }
} // namespace lve