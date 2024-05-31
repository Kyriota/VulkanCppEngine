#include "fluid_particle_system.hpp"

// std
#include <algorithm>

namespace lve
{
    FluidParticleSystem::FluidParticleSystem(ParticleSysInitData initData)
        : particleCount(initData.particleCount),
          windowExtent(initData.windowExtent),
          smoothRadius(initData.smoothRadius),
          collisionDamping(initData.collisionDamping),
          targetDensity(initData.targetDensity)
    {
        particleData = initParticleData(initData.startPoint, initData.stride, initData.maxWidth);
    }

    FluidParticleSystem::ParticleData FluidParticleSystem::initParticleData(glm::vec2 startPoint, float stride, float maxWidth)
    {
        ParticleData particleBufferData{};
        particleBufferData.numParticles = particleCount;
        particleBufferData.smoothingRadius = smoothRadius;
        particleBufferData.positions.resize(particleCount);
        particleBufferData.velocities.resize(particleCount);

        maxWidth -= std::fmod(maxWidth, stride);
        int cntPerRow = static_cast<int>(maxWidth / stride);
        int row, col;
        for (int i = 0; i < particleCount; i++)
        {
            row = static_cast<int>(i / cntPerRow);
            col = i % cntPerRow;

            // random position inside window
            particleBufferData.positions[i] = glm::vec2(
                static_cast<float>(rand() % static_cast<int>(windowExtent.width)),
                static_cast<float>(rand() % static_cast<int>(windowExtent.height)));

            // particleBufferData.positions[i] = startPoint + glm::vec2(col * stride, row * stride);

            // random velocity
            // particleBufferData.velocities[i] = glm::vec2(
            //     static_cast<float>(rand() % 400) - 200.f,
            //     static_cast<float>(rand() % 400) - 200.f);

            particleBufferData.velocities[i] = glm::vec2(0.f, 0.f);
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

        handleBoundaryCollision();
    }

    float FluidParticleSystem::kernelPoly6(float radius, float distance)
    {
        if (distance >= radius)
        {
            return 0.f;
        }
        return 315.f / (64.f * M_PI * std::pow(radius, 9)) * std::pow(radius * radius - distance * distance, 3);
    }

    float FluidParticleSystem::kernelSpiky(float radius, float distance)
    {
        if (distance >= radius)
        {
            return 0.f;
        }
        return -45.f / (M_PI * std::pow(radius, 6)) * (radius - distance) * (radius - distance);
    }

    void FluidParticleSystem::handleBoundaryCollision()
    {
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