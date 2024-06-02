#include "fluid_particle_system.hpp"
#include "lve/lve_math.hpp"
#include "lve/lve_file_io.hpp"

// std
#include <algorithm>

namespace lve
{
    FluidParticleSystem::FluidParticleSystem(const std::string &configFilePath, VkExtent2D windowExtent) : windowExtent(windowExtent)
    {
        LveYamlConfig config{configFilePath};

        particleCount = config.get<unsigned int>("particleCount");
        smoothRadius = config.get<float>("smoothRadius");
        collisionDamping = config.get<float>("collisionDamping");
        targetDensity = config.get<float>("targetDensity");
        pressureMultiplier = config.get<float>("pressureMultiplier");

        std::vector<float> startPoint = config.get<std::vector<float>>("startPoint");
        float stride = config.get<float>("stride");
        float maxWidth = config.get<float>("maxWidth");

        initParticleData(glm::vec2(startPoint[0], startPoint[1]), stride, maxWidth);
        updateDensities();
    }

    void FluidParticleSystem::initParticleData(glm::vec2 startPoint, float stride, float maxWidth)
    {
        dataPosition.resize(particleCount);
        dataVelocity.resize(particleCount);
        dataDensity.resize(particleCount);
        dataMass.resize(particleCount);

        maxWidth -= std::fmod(maxWidth, stride);
        int cntPerRow = static_cast<int>(maxWidth / stride);
        int row, col;
        for (int i = 0; i < particleCount; i++)
        {
            row = static_cast<int>(i / cntPerRow);
            col = i % cntPerRow;

            // random position inside window
            dataPosition[i] = glm::vec2(
                static_cast<float>(rand() % static_cast<int>(windowExtent.width)),
                static_cast<float>(rand() % static_cast<int>(windowExtent.height)));

            // dataPosition[i] = startPoint + glm::vec2(col * stride, row * stride);

            // random velocity
            // dataVelocity[i] = glm::vec2(
            //     static_cast<float>(rand() % 400) - 200.f,
            //     static_cast<float>(rand() % 400) - 200.f);

            dataVelocity[i] = glm::vec2(0.f, 0.f);

            dataMass[i] = 1.f;
        }
    }

    void FluidParticleSystem::updateParticleData(float deltaTime)
    {
        // particle update rule
        for (int i = 0; i < particleCount; i++)
        {
            dataPosition[i] += dataVelocity[i] * deltaTime;
        }

        handleBoundaryCollision();
    }

    float FluidParticleSystem::kernelPoly6_2D(float distance)
    {
        if (distance >= smoothRadius)
            return 0.f;
        float v = smoothRadius * smoothRadius - distance * distance;
        return scalingFactorPoly6_2D * v * v * v;
    }

    float FluidParticleSystem::kernelSpikyPow3_2D(float distance)
    {
        if (distance >= smoothRadius)
            return 0.f;
        float v = smoothRadius - distance;
        return scalingFactorSpikyPow3_2D * v * v * v;
    }

    float FluidParticleSystem::derivativeSpikyPow3_2D(float distance)
    {
        if (distance >= smoothRadius)
            return 0.f;
        float v = smoothRadius - distance;
        return -3.f * scalingFactorSpikyPow3_2D * v * v;
    }

    float FluidParticleSystem::kernelSpikyPow2_2D(float distance)
    {
        if (distance >= smoothRadius)
            return 0.f;
        float v = smoothRadius - distance;
        return scalingFactorSpikyPow2_2D * v * v;
    }

    float FluidParticleSystem::derivativeSpikyPow2_2D(float distance)
    {
        if (distance >= smoothRadius)
            return 0.f;
        float v = smoothRadius - distance;
        return -2.f * scalingFactorSpikyPow2_2D * v;
    }

    void FluidParticleSystem::updateDensities()
    {
        for (int i = 0; i < particleCount; i++)
        {
            dataDensity[i] = calculateDensity(dataPosition[i]);
        }
    }

    float FluidParticleSystem::calculateDensity(glm::vec2 samplePos)
    {
        float density = 0.f;
        for (int i = 0; i < particleCount; i++)
        {
            float distance = glm::distance(samplePos, dataPosition[i]);
            float influence = kernelPoly6_2D(distance);
            density += dataMass[i] * influence;
        }
        return density;
    }
    
    glm::vec2 FluidParticleSystem::calculateDensityGradient(glm::vec2 samplePos)
    {
        glm::vec2 gradient = glm::vec2(0.f, 0.f);

        for (int i = 0; i < particleCount; i++)
        {
            float distance = glm::distance(samplePos, dataPosition[i]);
            glm::vec2 dir = glm::normalize(samplePos - dataPosition[i]);
            float derivative = derivativeSpikyPow3_2D(distance);
            gradient += dataMass[i] * derivative * dir;
        }

        return gradient;
    }

    void FluidParticleSystem::handleBoundaryCollision()
    {
        for (int i = 0; i < particleCount; i++)
        {
            if (dataPosition[i].x < 0 || dataPosition[i].x > windowExtent.width)
            {
                dataPosition[i].x = std::clamp(dataPosition[i].x, 0.f, static_cast<float>(windowExtent.width));
                dataVelocity[i].x *= -collisionDamping;
            }
            if (dataPosition[i].y < 0 || dataPosition[i].y > windowExtent.height)
            {
                dataPosition[i].y = std::clamp(dataPosition[i].y, 0.f, static_cast<float>(windowExtent.height));
                dataVelocity[i].y *= -collisionDamping;
            }
        }
    }
} // namespace lve