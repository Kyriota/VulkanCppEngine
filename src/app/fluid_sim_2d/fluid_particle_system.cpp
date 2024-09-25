#include "app/fluid_sim_2d/fluid_particle_system.hpp"

#include "lve/path.hpp"
#include "lve/util/config.hpp"
#include "lve/util/file_io.hpp"
#include "lve/util/math.hpp"

// std
#include <algorithm>
#include <iostream>

namespace app::fluidsim2d
{
FluidParticleSystem::FluidParticleSystem(VkExtent2D windowExtent) : windowExtent(windowExtent)
{
    const lve::YamlConfig config = lve::ConfigManager::getConfig(lve::path::config::FLUID_SIM_2D);
    particleCount = config.get<size_t>("particleCount");

    initSimParams();

    std::vector<float> startPoint = config.get<std::vector<float>>("startPoint");
    float stride = config.get<float>("stride");
    float maxWidth = config.get<float>("maxWidth");
    bool randomize = config.get<bool>("randomize");

    initParticleData(glm::vec2(startPoint[0], startPoint[1]), stride, maxWidth, randomize);
}

void FluidParticleSystem::reloadConfigParam()
{
    lve::ConfigManager::reloadConfig(lve::path::config::FLUID_SIM_2D);
    initSimParams();
}

void FluidParticleSystem::initParticleData(
    glm::vec2 startPoint, float stride, float maxWidth, bool randomize)
{
    positionData.resize(particleCount);
    nextPositionData.resize(particleCount);
    velocityData.resize(particleCount);
    densityData.resize(particleCount);
    massData.resize(particleCount);

    spacialLookup.resize(particleCount);
    spacialLookupEntry.resize(particleCount);

    // debug
    pressureForceData.resize(particleCount);
    externalForceData.resize(particleCount);
    viscosityForceData.resize(particleCount);
    firstParticleNeighborIndex.resize(particleCount);
    debugLines.resize(particleCount);
    for (int i = 0; i < particleCount; i++)
    {
        debugLines[i].start.color = glm::vec4(1.f, 0.f, 0.f, 1.f);
        debugLines[i].end.color = glm::vec4(0.f, 1.f, 0.f, 1.f);
    }

    // init particle data
    int cntPerRow = static_cast<int>(maxWidth / stride);
    maxWidth -= std::fmod(maxWidth, stride);
    int row, col;
    for (int i = 0; i < particleCount; i++)
    {
        row = static_cast<int>(i / cntPerRow);
        col = i % cntPerRow;

        if (randomize)
            positionData[i] = glm::vec2(
                static_cast<float>(rand()) / static_cast<float>(RAND_MAX / scaledWindowExtent.x),
                static_cast<float>(rand()) / static_cast<float>(RAND_MAX / scaledWindowExtent.y));
        else
            positionData[i] = startPoint + glm::vec2(col * stride, row * stride);

        velocityData[i] = glm::vec2(0.f, 0.f);

        massData[i] = 1.f;
    }
}

void FluidParticleSystem::initSimParams()
{
    lve::YamlConfig config = lve::ConfigManager::getConfig(lve::path::config::FLUID_SIM_2D);

    smoothRadius = config.get<float>("smoothRadius");
    boundaryMultipler = config.get<float>("boundaryMultipler");
    targetDensity = config.get<float>("targetDensity");
    pressureMultiplier = config.get<float>("pressureMultiplier");
    nearPressureMultiplier = config.get<float>("nearPressureMultiplier");
    viscosityMultiplier = config.get<float>("viscosityMultiplier");
    gravityAccValue = config.get<float>("gravityAccValue");
    dataScale = config.get<float>("dataScale");
    rangeForceScale = config.get<float>("rangeForceScale");
    rangeForceRadius = config.get<float>("rangeForceRadius");

    scaledWindowExtent.x = static_cast<float>(windowExtent.width) * dataScale;
    scaledWindowExtent.y = static_cast<float>(windowExtent.height) * dataScale;

    // init kernel constants
    scalingFactorPoly6_2D = 4.f / (M_PI * lve::math::intPow(smoothRadius, 8));
    scalingFactorSpikyPow3_2D = 10.f / (M_PI * lve::math::intPow(smoothRadius, 5));
    scalingFactorSpikyPow2_2D = 6.f / (M_PI * lve::math::intPow(smoothRadius, 4));
    scalingFactorPoly6_2D_atZero = kernelPoly6_2D(0.f, smoothRadius);
    scalingFactorSpikyPow3_2D_atZero = kernelSpikyPow3_2D(0.f, smoothRadius);
    scalingFactorSpikyPow2_2D_atZero = kernelSpikyPow2_2D(0.f, smoothRadius);
}

glm::vec2 FluidParticleSystem::scaledPos2ScreenPos(glm::vec2 scaledPos) const
{
    return 2.f * scaledPos / scaledWindowExtent - glm::vec2(1.f, 1.f);
}

void FluidParticleSystem::updateWindowExtent(VkExtent2D newExtent)
{
    windowExtent = newExtent;
    scaledWindowExtent.x = static_cast<float>(windowExtent.width) * dataScale;
    scaledWindowExtent.y = static_cast<float>(windowExtent.height) * dataScale;
}

void FluidParticleSystem::updateParticleData(float deltaTime)
{
    if (isPaused)
    {
        if (!pausedNextFrame)
            return;
        std::cout << "Render next frame" << std::endl;
        deltaTime = maxDeltaTime;
        pausedNextFrame = false;
    }

    if (deltaTime > maxDeltaTime)
        deltaTime = maxDeltaTime;

    for (int i = 0; i < particleCount; i++) // update predicted position and spacial lookup
    {
        nextPositionData[i] = positionData[i] + velocityData[i] * lookAheadTime;
        int hashValue = hashGridCoord2D(pos2gridCoord(nextPositionData[i], smoothRadius));
        unsigned int hashKey = lve::math::positiveMod(hashValue, particleCount);
        spacialLookup[i].particleIndex = i;
        spacialLookup[i].spatialHashKey = hashKey;
    }

    std::sort(
        spacialLookup.begin(),
        spacialLookup.end(),
        [](const SpatialHashEntry &a, const SpatialHashEntry &b) {
            return a.spatialHashKey < b.spatialHashKey;
        });

    // init spacial lookup entry
    std::fill(spacialLookupEntry.begin(), spacialLookupEntry.end(), -1);
    for (int i = 0; i < particleCount; i++)
    {
        unsigned int key = spacialLookup[i].spatialHashKey;
        unsigned int keyPrev = (i == 0) ? -1 : spacialLookup[i - 1].spatialHashKey;
        if (key != keyPrev)
            spacialLookupEntry[key] = i;
    }

    if (isNeighborViewActive)
    {
        // store neighbor index of the first particle
        firstParticleNeighborIndex.clear();
        foreachNeighbor(0, [&](int neighborIndex) {
            firstParticleNeighborIndex.push_back(neighborIndex);
        });
        firstParticleNeighborIndex.push_back(-1); // mark the end of the list
    }

    for (int i = 0; i < particleCount; i++) // calculate density using predicted position
        densityData[i] = calculateDensity(i);

    for (int i = 0; i < particleCount; i++) // update velocity and position
    {
        pressureForceData[i] = calculatePressureForce(i);
        externalForceData[i] = calculateExternalForce(i);
        viscosityForceData[i] = calculateViscosityForce(i);

        glm::vec2 acceleration =
            (pressureForceData[i] + viscosityForceData[i] + externalForceData[i]) /
            densityData[i].density;
        velocityData[i] += acceleration * deltaTime;
        positionData[i] += velocityData[i] * deltaTime;
    }

    rangeForceInfo.active = false;

    // update debug lines
    if (isDebugLineVisible)
        updateDebugLines();
}

void FluidParticleSystem::updateDebugLines()
{
    auto setDebugLines = [&](std::function<glm::vec2(int)> callback) {
        for (int i = 0; i < particleCount; i++)
        {
            glm::vec2 particlePos = positionData[i];
            debugLines[i].start.position = glm::vec3(scaledPos2ScreenPos(particlePos), debugLineZ);
            debugLines[i].end.position =
                glm::vec3(scaledPos2ScreenPos(particlePos + callback(i)), debugLineZ);
        }
    };

    if (debugLineType == VELOCITY)
        setDebugLines([&](int i) {
            return velocityData[i] * 0.1f;
        });
    else if (debugLineType == PRESSURE_FORCE)
        setDebugLines([&](int i) {
            return pressureForceData[i] / (pressureMultiplier + nearPressureMultiplier) * 0.05f;
        });
    else if (debugLineType == EXTERNAL_FORCE)
        setDebugLines([&](int i) {
            return externalForceData[i] *
                lve::math::fastInvSqrt(glm::length(externalForceData[i])) * 0.01f;
        });
}

void FluidParticleSystem::setRangeForcePos(bool isRepulsive, glm::vec2 mousePosition)
{
    rangeForceInfo.active = true;
    rangeForceInfo.isRepulsive = isRepulsive;
    rangeForceInfo.position = mousePosition * dataScale;
}

float FluidParticleSystem::kernelPoly6_2D(float distance, float radius) const
{
    if (distance >= radius)
        return 0.f;
    float v = radius * radius - distance * distance;
    return scalingFactorPoly6_2D * v * v * v;
}

float FluidParticleSystem::kernelSpikyPow3_2D(float distance, float radius) const
{
    if (distance >= radius)
        return 0.f;
    float v = radius - distance;
    return scalingFactorSpikyPow3_2D * v * v * v;
}

float FluidParticleSystem::derivativeSpikyPow3_2D(float distance, float radius) const
{
    if (distance >= radius)
        return 0.f;
    float v = radius - distance;
    return -3.f * scalingFactorSpikyPow3_2D * v * v;
}

float FluidParticleSystem::kernelSpikyPow2_2D(float distance, float radius) const
{
    if (distance >= radius)
        return 0.f;
    float v = radius - distance;
    return scalingFactorSpikyPow2_2D * v * v;
}

float FluidParticleSystem::derivativeSpikyPow2_2D(float distance, float radius) const
{
    if (distance >= radius)
        return 0.f;
    float v = radius - distance;
    return -2.f * scalingFactorSpikyPow2_2D * v;
}

FluidParticleSystem::Density FluidParticleSystem::calculateDensity(size_t particleIndex)
{
    float density = massData[particleIndex] * scalingFactorSpikyPow2_2D_atZero;
    float nearDensity = massData[particleIndex] * scalingFactorSpikyPow3_2D_atZero;
    glm::vec2 particleNextPos = nextPositionData[particleIndex];
    foreachNeighbor(particleIndex, [&](int neighborIndex) {
        float distance = glm::distance(particleNextPos, nextPositionData[neighborIndex]);
        if (distance >= smoothRadius)
            return;
        density += massData[neighborIndex] * kernelSpikyPow2_2D(distance, smoothRadius);
        nearDensity += massData[neighborIndex] * kernelSpikyPow3_2D(distance, smoothRadius);
    });
    return {density, nearDensity};
}

glm::vec2 FluidParticleSystem::calculatePressureForce(size_t particleIndex)
{
    glm::vec2 pressureForce = glm::vec2(0.f, 0.f);
    glm::vec2 particleNextPos = nextPositionData[particleIndex];
    float pressureThis = pressureMultiplier * (densityData[particleIndex].density - targetDensity);
    float nearPressureThis = nearPressureMultiplier * densityData[particleIndex].nearDensity;

    foreachNeighbor(particleIndex, [&](int neighborIndex) {
        float distance = glm::distance(particleNextPos, nextPositionData[neighborIndex]);
        if (distance >= smoothRadius)
            return;

        glm::vec2 dir;
        if (distance < glm::epsilon<float>())
            dir = glm::circularRand(1.f);
        else
            dir = glm::normalize(nextPositionData[neighborIndex] - particleNextPos);

        float pressureOther =
            pressureMultiplier * (densityData[neighborIndex].density - targetDensity);
        float nearPressureOther = nearPressureMultiplier * densityData[neighborIndex].nearDensity;
        float sharedPressure = (pressureThis + pressureOther) * 0.5f;
        float sharedNearPressure = (nearPressureThis + nearPressureOther) * 0.5f;
        pressureForce += derivativeSpikyPow2_2D(distance, smoothRadius) /
            densityData[neighborIndex].density * sharedPressure * dir;
        pressureForce += derivativeSpikyPow3_2D(distance, smoothRadius) /
            densityData[neighborIndex].nearDensity * sharedNearPressure * dir;
    });
    return pressureForce;
}

glm::vec2 FluidParticleSystem::calculateExternalForce(size_t particleIndex)
{
    glm::vec2 externalForce = glm::vec2(0.f, 0.f);

    // boundary force, push particles back to range when they are near the
    // boundary
    glm::vec2 particleNextPos = nextPositionData[particleIndex];
    bool outOfX = particleNextPos.x < boundaryMargin ||
        particleNextPos.x > scaledWindowExtent.x - boundaryMargin;
    bool outOfY = particleNextPos.y < boundaryMargin ||
        particleNextPos.y > scaledWindowExtent.y - boundaryMargin;
    if (outOfX || outOfY)
    {
        glm::vec2 boundaryForce = glm::vec2(0.f, 0.f);
        if (outOfX)
            boundaryForce.x = (particleNextPos.x < boundaryMargin)
                ? boundaryMargin - particleNextPos.x
                : scaledWindowExtent.x - boundaryMargin - particleNextPos.x;
        if (outOfY)
            boundaryForce.y = (particleNextPos.y < boundaryMargin)
                ? boundaryMargin - particleNextPos.y
                : scaledWindowExtent.y - boundaryMargin - particleNextPos.y;

        // slow down the velocity when particles are out of boundary
        externalForce +=
            boundaryMultipler * (boundaryForce - velocityData[particleIndex] * dataScale);
    }

    // gravity
    glm::vec2 gravityForce = glm::vec2(0.f, gravityAccValue * densityData[particleIndex].density);
    externalForce += gravityForce;

    // range force
    if (rangeForceInfo.active)
    {
        glm::vec2 particlePos = positionData[particleIndex];
        float distance = glm::distance(particlePos, rangeForceInfo.position);
        if (distance < rangeForceRadius && distance > glm::epsilon<float>())
        {
            glm::vec2 dir = glm::normalize(rangeForceInfo.position - particlePos);
            if (rangeForceInfo.isRepulsive)
            {
                float repulsiveRadius = rangeForceRadius * 0.75f;
                if (distance < repulsiveRadius)
                {
                    float distOverRadius = distance / repulsiveRadius; // 0 at center, 1 at edge
                    externalForce -= 2.5f * lve::math::fastSqrt(1 - distOverRadius) *
                        rangeForceScale * densityData[particleIndex].density * dir;
                }
            }
            else
            {
                float distOverRadius = distance / rangeForceRadius; // 0 at center, 1 at edge
                float rangeForceDistMultiplier = distance > rangeForceRadius * 0.5f
                    ? lve::math::fastSqrt(1 - distOverRadius)
                    : lve::math::fastSqrt(distOverRadius);
                externalForce += rangeForceDistMultiplier * rangeForceScale *
                    densityData[particleIndex].density * dir;

                // slow down the velocity when particles are in the range
                externalForce -= rangeForceScale * viscosityMultiplier * (1 - distOverRadius) *
                    densityData[particleIndex].density * velocityData[particleIndex];
            }
        }
    }

    return externalForce;
}

glm::vec2 FluidParticleSystem::calculateViscosityForce(size_t particleIndex)
{
    glm::vec2 viscosityForce = glm::vec2(0.f, 0.f);
    glm::vec2 particleNextPos = nextPositionData[particleIndex];
    foreachNeighbor(particleIndex, [&](int neighborIndex) {
        float distance = glm::distance(particleNextPos, nextPositionData[neighborIndex]);
        if (distance >= smoothRadius)
            return;

        glm::vec2 relativeVelocity = velocityData[neighborIndex] - velocityData[particleIndex];
        viscosityForce += relativeVelocity * kernelPoly6_2D(distance, smoothRadius);
    });
    return viscosityForce * viscosityMultiplier;
}

glm::int2 FluidParticleSystem::pos2gridCoord(glm::vec2 position, float gridWidth) const
{
    int x = static_cast<int>(position.x / gridWidth);
    int y = static_cast<int>(position.y / gridWidth);
    return {x, y};
}

int FluidParticleSystem::hashGridCoord2D(glm::int2 gridCoord) const
{
    return static_cast<uint32_t>(gridCoord.x) * 15823 +
        static_cast<uint32_t>(gridCoord.y) * 9737333;
}

/*
 * Iterate over all neighbors of a particle, excluding itself
 * @param particleIndex: index of the particle
 * @param callback: function to be called for each neighbor
 */
void FluidParticleSystem::foreachNeighbor(size_t particleIndex, std::function<void(int)> callback)
{
    glm::vec2 particleNextPos = nextPositionData[particleIndex];
    glm::int2 gridPos = pos2gridCoord(particleNextPos, smoothRadius);
    float smoothRadius_mul_2 = 2.f * smoothRadius;
    for (int i = 0; i < 9; i++)
    {
        glm::int2 offsetGridPos = gridPos + offset2D[i];
        unsigned int hashKey =
            lve::math::positiveMod(hashGridCoord2D(offsetGridPos), particleCount);
        int startIndex = spacialLookupEntry[hashKey];
        if (startIndex == -1) // no particle in this grid
            continue;

        for (int j = startIndex; j < particleCount; j++)
        {
            if (spacialLookup[j].spatialHashKey != hashKey)
                break;

            size_t neighborIndex = spacialLookup[j].particleIndex;

            // simple check to skip hash collision
            glm::vec2 neighborNextPos = nextPositionData[neighborIndex];
            if (std::abs(neighborNextPos.x - particleNextPos.x) > smoothRadius_mul_2 ||
                std::abs(neighborNextPos.y - particleNextPos.y) > smoothRadius_mul_2)
                continue;

            if (neighborIndex != particleIndex)
                callback(neighborIndex);
        }
    }
}
} // namespace app::fluidsim2d