#pragma once

// libs
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

// std
#include <vector>

namespace lve
{
    class FluidParticleSystem
    {
    public:
        struct ParticleData
        {
            unsigned int numParticles;
            std::vector<glm::vec2> positions;
            std::vector<glm::vec2> velocities;
        };

        struct ParticleSysInitData
        {
            unsigned int particleCount;

            // fluid parameters
            float smoothRadius = 2.0f;
            float collisionDamping = 0.9f;
            float targetDensity = 1.0f;

            // particle initialization parameters
            glm::vec2 startPoint = glm::vec2(300.f, 300.f);
            float stride = 10.f;
            float maxWidth = 105.f;
            VkExtent2D windowExtent;
        };

        FluidParticleSystem(ParticleSysInitData initData)
            : particleCount(initData.particleCount),
              smoothRadius(initData.smoothRadius),
              collisionDamping(initData.collisionDamping),
              targetDensity(initData.targetDensity),
              windowExtent(initData.windowExtent),
              particleData(initParticleData(initData.startPoint, initData.stride, initData.maxWidth)) {}

        unsigned int getParticleCount() { return particleCount; }
        ParticleData &getParticleData() { return particleData; }

        void updateParticleData(float deltaTime);

    private:
        unsigned int particleCount;
        VkExtent2D windowExtent;

        ParticleData particleData;
        float smoothRadius;
        float collisionDamping;
        float targetDensity;

        ParticleData initParticleData(glm::vec2 startPoint, float stride, float maxWidth);
    };
} // namespace lve