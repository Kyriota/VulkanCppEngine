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
            float smoothingRadius;
            std::vector<glm::vec2> positions;
            std::vector<glm::vec2> velocities;
        };

        struct ParticleSysInitData
        {
            unsigned int particleCount;

            // fluid parameters
            float smoothRadius = 50.0f;
            float collisionDamping = 0.9f;
            float targetDensity = 1.0f;

            // particle initialization parameters
            glm::vec2 startPoint = glm::vec2(300.f, 300.f);
            float stride = 10.f;
            float maxWidth = 105.f;
            VkExtent2D windowExtent;
        };

        FluidParticleSystem(ParticleSysInitData initData);

        ParticleData &getParticleData() { return particleData; }

        void updateParticleData(float deltaTime);
        void updateWindowExtent(VkExtent2D newExtent) { windowExtent = newExtent; }

    private:
        unsigned int particleCount;
        VkExtent2D windowExtent;

        ParticleData particleData;
        float smoothRadius;
        float collisionDamping;
        float targetDensity;

        ParticleData initParticleData(glm::vec2 startPoint, float stride, float maxWidth);

        // update rules
        float kernelPoly6(float radius, float distance);
        float kernelSpiky(float radius, float distance);
        float calculateDensity(glm::vec2 samplePos);
        void handleBoundaryCollision();
    };
} // namespace lve