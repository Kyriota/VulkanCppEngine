#pragma once

#define M_PI 3.14159265358979323846

#include "lve/lve_math.hpp"

// libs
#include "include/glm.hpp"
#include <vulkan/vulkan.h>

// std
#include <string>
#include <vector>

namespace lve
{
    class FluidParticleSystem
    {
    public:
        FluidParticleSystem(const std::string &configFilePath, VkExtent2D windowExtent);

        void updateParticleData(float deltaTime);
        void updateWindowExtent(VkExtent2D newExtent) { windowExtent = newExtent; }

        float getParticleCount() const { return particleCount; }
        float getSmoothRadius() const { return smoothRadius; }
        float getTargetDensity() const { return targetDensity; }
        std::vector<glm::vec2> &getPositionData() { return dataPosition; }
        std::vector<glm::vec2> &getVelocityData() { return dataVelocity; }

    private:
        unsigned int particleCount;
        VkExtent2D windowExtent;

        float smoothRadius;
        float collisionDamping;
        float targetDensity;
        float pressureMultiplier;

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