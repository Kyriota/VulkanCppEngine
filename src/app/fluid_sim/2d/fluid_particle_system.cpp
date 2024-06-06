#include "app/fluid_sim/2d/fluid_particle_system.hpp"
#include "lve/util/math.hpp"
#include "lve/util/file_io.hpp"

// std
#include <algorithm>
#include <iostream>

FluidParticleSystem::FluidParticleSystem(const std::string &configFilePath, VkExtent2D windowExtent) : windowExtent(windowExtent)
{
    this->configFilePath = configFilePath;
    lve::io::YamlConfig config{configFilePath};
    particleCount = config.get<unsigned int>("particleCount");

    initSimParams(config);

    std::vector<float> startPoint = config.get<std::vector<float>>("startPoint");
    float stride = config.get<float>("stride");
    float maxWidth = config.get<float>("maxWidth");
    bool randomize = config.get<bool>("randomize");

    initParticleData(glm::vec2(startPoint[0], startPoint[1]), stride, maxWidth, randomize);
}

void FluidParticleSystem::reloadConfigParam()
{
    lve::io::YamlConfig config{configFilePath};
    initSimParams(config);
}

void FluidParticleSystem::initParticleData(glm::vec2 startPoint, float stride, float maxWidth, bool randomize)
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
            positionData[i] = glm::vec2(static_cast<float>(rand()) / static_cast<float>(RAND_MAX / scaledWindowExtent.x),
                                        static_cast<float>(rand()) / static_cast<float>(RAND_MAX / scaledWindowExtent.y));
        else
            positionData[i] = startPoint + glm::vec2(col * stride, row * stride);

        velocityData[i] = glm::vec2(0.f, 0.f);

        massData[i] = 1.f;
    }
}

void FluidParticleSystem::initSimParams(lve::io::YamlConfig &config)
{
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
        return;
    
    if (deltaTime > maxDeltaTime)
        deltaTime = maxDeltaTime;

    for (int i = 0; i < particleCount; i++) // predict position
        nextPositionData[i] = positionData[i] + velocityData[i] * lookAheadTime;

    updateSpatialLookup();

    // store neighbor index of the first particle for debug
    firstParticleNeighborIndex.clear();
    foreachNeighbor(0, [&](int neighborIndex)
                    { firstParticleNeighborIndex.push_back(neighborIndex); });
    firstParticleNeighborIndex.push_back(-1); // mark the end of the list

    for (int i = 0; i < particleCount; i++) // calculate density using predicted position
        densityData[i] = calculateDensity(i);

    for (int i = 0; i < particleCount; i++) // update velocity and position
    {
        pressureForceData[i] = calculatePressureForce(i);
        externalForceData[i] = calculateExternalForce(i);
        viscosityForceData[i] = calculateViscosityForce(i);

        glm::vec2 acceleration = (pressureForceData[i] + viscosityForceData[i] + externalForceData[i]) / densityData[i];
        velocityData[i] += acceleration * deltaTime;
        positionData[i] += velocityData[i] * deltaTime;
    }

    rangeForceInfo.active = false;

    // update debug lines
    for (int i = 0; i < particleCount; i++)
    {
        glm::vec2 particlePos = positionData[i];
        debugLines[i].start.position = glm::vec3(scaledPos2ScreenPos(particlePos), 0.f);
        // draw velocity
        // debugLines[i].end.position = glm::vec3(scaledPos2ScreenPos(particlePos + velocityData[i] * 0.1f), 0.f);
        // draw pressure force
        debugLines[i].end.position = glm::vec3(scaledPos2ScreenPos(particlePos + pressureForceData[i] / pressureMultiplier * 0.1f), 0.f);
    }
}

void FluidParticleSystem::setRangeForcePos(bool sign, glm::vec2 mousePosition)
{
    rangeForceInfo.active = true;
    rangeForceInfo.sign = sign;
    rangeForceInfo.position = mousePosition * dataScale;
}

void FluidParticleSystem::printDensity(glm::vec2 mousePosition)
{
    unsigned int minIndex = getClosetParticleIndex(mousePosition);
    std::cout << "Density[" << minIndex << "]: " << densityData[minIndex] << std::endl;
}

void FluidParticleSystem::printPressureForce(glm::vec2 mousePosition)
{
    unsigned int minIndex = getClosetParticleIndex(mousePosition);
    glm::vec2 pressureForce = calculatePressureForce(minIndex);
    std::cout << "Pressure[" << minIndex << "]: (" << pressureForce.x << ", " << pressureForce.y << "), \t magnitude: " << glm::length(pressureForce) << std::endl;
}

unsigned int FluidParticleSystem::getClosetParticleIndex(glm::vec2 mousePosition)
{
    glm::vec2 position = mousePosition * dataScale;
    float minDistance = std::numeric_limits<float>::max();
    int minIndex = -1;
    for (int i = 0; i < particleCount; i++)
    {
        float distance = glm::distance(positionData[i], position);
        if (distance < minDistance)
        {
            minDistance = distance;
            minIndex = i;
        }
    }
    return minIndex;
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

float FluidParticleSystem::calculateDensity(unsigned int particleIndex)
{
    float density = massData[particleIndex] * scalingFactorSpikyPow2_2D_atZero;
    glm::vec2 particleNextPos = nextPositionData[particleIndex];
    foreachNeighbor(
        particleIndex,
        [&](int neighborIndex)
        {
            float distance = glm::distance(particleNextPos, nextPositionData[neighborIndex]);
            if (distance >= smoothRadius)
                return;
            density += massData[neighborIndex] * kernelSpikyPow2_2D(distance, smoothRadius);
        });
    return density;
}

glm::vec2 FluidParticleSystem::calculatePressureForce(unsigned int particleIndex)
{
    glm::vec2 pressureForce = glm::vec2(0.f, 0.f);
    glm::vec2 particleNextPos = nextPositionData[particleIndex];
    float pressureThis = pressureMultiplier * (densityData[particleIndex] - targetDensity);

    foreachNeighbor(
        particleIndex,
        [&](int neighborIndex)
        {
            float distance = glm::distance(particleNextPos, nextPositionData[neighborIndex]);
            if (distance >= smoothRadius)
                return;

            glm::vec2 dir;
            if (distance < glm::epsilon<float>())
                dir = glm::circularRand(1.f);
            else
                dir = glm::normalize(nextPositionData[neighborIndex] - particleNextPos);

            float pressureOther = pressureMultiplier * (densityData[neighborIndex] - targetDensity);
            float sharedPressure = (pressureThis + pressureOther) * 0.5f;
            pressureForce += derivativeSpikyPow2_2D(distance, smoothRadius) /
                             densityData[neighborIndex] * sharedPressure * dir;
        });
    return pressureForce;
}

glm::vec2 FluidParticleSystem::calculateExternalForce(unsigned int particleIndex)
{
    glm::vec2 externalForce = glm::vec2(0.f, 0.f);

    // range force
    if (rangeForceInfo.active)
    {
        glm::vec2 particlePos = positionData[particleIndex];
        float distance = glm::distance(particlePos, rangeForceInfo.position);
        if (distance < rangeForceRadius && distance > glm::epsilon<float>())
        {
            glm::vec2 dir = glm::normalize(rangeForceInfo.position - particlePos);
            if (rangeForceInfo.sign)
                dir *= -1.f;
            externalForce += rangeForceScale * dir * (rangeForceRadius - distance) / rangeForceRadius;
        }
    }

    // boundary force, push particles back to range when they are near the boundary
    glm::vec2 particleNextPos = nextPositionData[particleIndex];
    bool outOfX = particleNextPos.x < boundaryMargin || particleNextPos.x > scaledWindowExtent.x - boundaryMargin;
    bool outOfY = particleNextPos.y < boundaryMargin || particleNextPos.y > scaledWindowExtent.y - boundaryMargin;
    if (outOfX || outOfY)
    {
        glm::vec2 boundaryForce = glm::vec2(0.f, 0.f);
        if (outOfX)
            boundaryForce.x = (particleNextPos.x < boundaryMargin) ? boundaryMargin - particleNextPos.x : scaledWindowExtent.x - boundaryMargin - particleNextPos.x;
        if (outOfY)
            boundaryForce.y = (particleNextPos.y < boundaryMargin) ? boundaryMargin - particleNextPos.y : scaledWindowExtent.y - boundaryMargin - particleNextPos.y;

        // slow down the velocity when particles are out of boundary
        externalForce += boundaryMultipler * (boundaryForce - velocityData[particleIndex] * dataScale);
    }

    // gravity
    externalForce += glm::vec2(0.f, gravityAccValue * densityData[particleIndex]);

    return externalForce;
}

glm::vec2 FluidParticleSystem::calculateViscosityForce(unsigned int particleIndex)
{
    glm::vec2 viscosityForce = glm::vec2(0.f, 0.f);
    glm::vec2 particleNextPos = nextPositionData[particleIndex];
    foreachNeighbor(
        particleIndex,
        [&](int neighborIndex)
        {
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
    return static_cast<uint32_t>(gridCoord.x) * 15823 + static_cast<uint32_t>(gridCoord.y) * 9737333;
}

void FluidParticleSystem::updateSpatialLookup()
{
    for (int i = 0; i < particleCount; i++)
    {
        int hashValue = hashGridCoord2D(pos2gridCoord(nextPositionData[i], smoothRadius));
        unsigned int hashKey = lve::math::positiveMod(hashValue, particleCount);
        spacialLookup[i].particleIndex = i;
        spacialLookup[i].spatialHashKey = hashKey;
    }

    std::sort(
        spacialLookup.begin(),
        spacialLookup.end(),
        [](const SpatialHashEntry &a, const SpatialHashEntry &b)
        {
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
}

/*
 * Iterate over all neighbors of a particle, excluding itself
 * @param particleIndex: index of the particle
 * @param callback: function to be called for each neighbor
 */
void FluidParticleSystem::foreachNeighbor(unsigned int particleIndex, std::function<void(int)> callback)
{
    glm::vec2 particleNextPos = nextPositionData[particleIndex];
    glm::int2 gridPos = pos2gridCoord(particleNextPos, smoothRadius);
    float smoothRadius_mul_2 = 2.f * smoothRadius;
    for (int i = 0; i < 9; i++)
    {
        glm::int2 offsetGridPos = gridPos + offset2D[i];
        unsigned int hashKey = lve::math::positiveMod(hashGridCoord2D(offsetGridPos), particleCount);
        int startIndex = spacialLookupEntry[hashKey];
        if (startIndex == -1) // no particle in this grid
            continue;

        for (int j = startIndex; j < particleCount; j++)
        {
            if (spacialLookup[j].spatialHashKey != hashKey)
                break;

            unsigned int neighborIndex = spacialLookup[j].particleIndex;

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