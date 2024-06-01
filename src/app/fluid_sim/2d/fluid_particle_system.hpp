#pragma once

#include "lve/lve_math.hpp"

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
        struct ParticleSysInitData
        {
            unsigned int particleCount;

            // fluid parameters
            float smoothRadius = 50.0f;
            float collisionDamping = 0.9f;
            float targetDensity = 0.001f;
            float pressureMultiplier = 1.0f;

            // particle initialization parameters
            glm::vec2 startPoint = glm::vec2(300.f, 300.f);
            float stride = 10.f;
            float maxWidth = 105.f;
            VkExtent2D windowExtent;
        };

        FluidParticleSystem(ParticleSysInitData initData);

        void updateParticleData(float deltaTime);
        void updateWindowExtent(VkExtent2D newExtent) { windowExtent = newExtent; }

        // getters
        const float getParticleCount() { return particleCount; }
        const float getSmoothRadius() { return smoothRadius; }
        const float getTargetDensity() { return targetDensity; }
        const std::vector<glm::vec2> &getPositionData() { return dataPosition; }
        const std::vector<glm::vec2> &getVelocityData() { return dataVelocity; }

    private:
        unsigned int particleCount;
        VkExtent2D windowExtent;

        float smoothRadius;
        float collisionDamping;
        float targetDensity;

        // particle data
        std::vector<glm::vec2> dataPosition;
        std::vector<glm::vec2> dataVelocity;
        std::vector<float> dataDensity;
        std::vector<float> dataMass;
        void initParticleData(glm::vec2 startPoint, float stride, float maxWidth);

        // kernels
        float kernelPoly6_2D(float distance);
        float scalingFactorPoly6_2D = 4.f / (M_PI * lveMath::intPow(smoothRadius, 8));
        float kernelSpikyPow3_2D(float distance);
        float derivativeSpikyPow3_2D(float distance);
        float scalingFactorSpikyPow3_2D = 10.f / (M_PI * lveMath::intPow(smoothRadius, 5));
        float kernelSpikyPow2_2D(float distance);
        float derivativeSpikyPow2_2D(float distance);
        float scalingFactorSpikyPow2_2D = 6.f / (M_PI * lveMath::intPow(smoothRadius, 4));

        // update rules
        void updateDensities();
        float calculateDensity(glm::vec2 samplePos);
        glm::vec2 calculateDensityGradient(glm::vec2 samplePos);
        void handleBoundaryCollision();
    };
} // namespace lve