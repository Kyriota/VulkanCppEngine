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

    maxWidth -= std::fmod(maxWidth, stride);
    int cntPerRow = static_cast<int>(maxWidth / stride);
    int row, col;
    for (int i = 0; i < particleCount; i++)
    {
        row = static_cast<int>(i / cntPerRow);
        col = i % cntPerRow;

        if (randomize)
            positionData[i] = glm::vec2(
                static_cast<float>(rand() % static_cast<int>(windowExtent.width)),
                static_cast<float>(rand() % static_cast<int>(windowExtent.height)));
        else
            positionData[i] = startPoint + glm::vec2(col * stride, row * stride);

        velocityData[i] = glm::vec2(0.f, 0.f);

        massData[i] = 1.f;
    }

    // data scaling
    for (int i = 0; i < particleCount; i++)
    {
        positionData[i] *= dataScale;
        velocityData[i] *= dataScale;
    }
}

void FluidParticleSystem::initSimParams(lve::io::YamlConfig &config)
{
    smoothRadius = config.get<float>("smoothRadius");
    collisionDamping = config.get<float>("collisionDamping");
    targetDensity = config.get<float>("targetDensity");
    pressureMultiplier = config.get<float>("pressureMultiplier");
    gravityAccValue = config.get<float>("gravityAccValue");
    dataScale = config.get<float>("dataScale");
    externalForceScale = config.get<float>("externalForceScale");
    externalForceRadius = config.get<float>("externalForceRadius");

    // init kernel constants
    scalingFactorPoly6_2D = 4.f / (M_PI * lve::math::intPow(smoothRadius, 8));
    scalingFactorSpikyPow3_2D = 10.f / (M_PI * lve::math::intPow(smoothRadius, 5));
    scalingFactorSpikyPow2_2D = 6.f / (M_PI * lve::math::intPow(smoothRadius, 4));
}

void FluidParticleSystem::updateParticleData(float deltaTime)
{
    for (int i = 0; i < particleCount; i++) // predict position
        nextPositionData[i] = positionData[i] + velocityData[i] * lookAheadTime;

    updateSpatialLookup();

    // calculate density using predicted position
    for (int i = 0; i < particleCount; i++)
        densityData[i] = calculateDensity(i);

    // update velocity
    for (int i = 0; i < particleCount; i++)
    {
        glm::vec2 pressureForce = calculatePressureForce(i);
        glm::vec2 externalForce = calculateExternalForce(i);
        glm::vec2 gravityAcc = glm::vec2(0.f, gravityAccValue * massData[i]); // positive gravity is down due to screen coordinates

        glm::vec2 acceleration = (pressureForce + externalForce) / densityData[i] + gravityAcc;
        velocityData[i] += acceleration * deltaTime;
    }

    // update position
    for (int i = 0; i < particleCount; i++)
        positionData[i] += velocityData[i] * deltaTime;

    handleBoundaryCollision();

    externalForceInfo.active = false;
}

void FluidParticleSystem::setExternalForcePos(bool sign, glm::vec2 position)
{
    externalForceInfo.active = true;
    externalForceInfo.sign = sign;
    externalForceInfo.position = position * dataScale;
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
    float density = massData[particleIndex] * kernelPoly6_2D(0.f, smoothRadius); // self density
    glm::vec2 particleNextPos = nextPositionData[particleIndex];
    foreachNeighbor(
        particleIndex,
        [&](int neighborIndex)
        {
            float distance = glm::distance(particleNextPos, nextPositionData[neighborIndex]);
            density += massData[neighborIndex] * kernelPoly6_2D(distance, smoothRadius);
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
            float sharedPressure = (pressureThis + pressureOther) / 2.f;
            pressureForce += sharedPressure *
                             derivativeSpikyPow2_2D(distance, smoothRadius) * massData[neighborIndex] /
                             densityData[neighborIndex] * dir;
        });
    return pressureForce;
}

glm::vec2 FluidParticleSystem::calculateExternalForce(unsigned int particleIndex)
{
    if (!externalForceInfo.active)
        return glm::vec2(0.f, 0.f);

    glm::vec2 externalForce = glm::vec2(0.f, 0.f);
    glm::vec2 particleNextPos = nextPositionData[particleIndex];
    float distance = glm::distance(particleNextPos, externalForceInfo.position);
    if (distance < externalForceRadius)
    {
        glm::vec2 dir = glm::normalize(externalForceInfo.position - particleNextPos);
        if (externalForceInfo.sign)
            dir *= -1.f;
        externalForce = externalForceScale * dir;
    }
    return externalForce;
}

void FluidParticleSystem::handleBoundaryCollision()
{
    for (int i = 0; i < particleCount; i++)
    {
        if (positionData[i].x < 0 || positionData[i].x > windowExtent.width * dataScale)
        {
            positionData[i].x = std::clamp(positionData[i].x, 0.f, static_cast<float>(windowExtent.width) * dataScale);
            velocityData[i].x *= -collisionDamping;
        }
        if (positionData[i].y < 0 || positionData[i].y > windowExtent.height * dataScale)
        {
            positionData[i].y = std::clamp(positionData[i].y, 0.f, static_cast<float>(windowExtent.height) * dataScale);
            velocityData[i].y *= -collisionDamping;
        }
    }
}

glm::int2 FluidParticleSystem::pos2gridCoord(glm::vec2 position, int gridWidth) const
{
    int x = position.x / gridWidth;
    int y = position.y / gridWidth;
    return {x, y};
}

int FluidParticleSystem::hashGridCoord2D(glm::int2 gridCoord) const
{
    return gridCoord.x * 73856093 ^ gridCoord.y * 83492791;
}

void FluidParticleSystem::updateSpatialLookup()
{
    for (int i = 0; i < particleCount; i++)
    {
        int hashValue = hashGridCoord2D(pos2gridCoord(nextPositionData[i], static_cast<int>(smoothRadius)));
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
    for (int i = 0; i < 9; i++)
    {
        glm::int2 offsetGridPos = gridPos + offset2D[i];
        unsigned int hashKey = lve::math::positiveMod(hashGridCoord2D(offsetGridPos), particleCount);
        int startIndex = spacialLookupEntry[hashKey];

        for (int j = startIndex; j < particleCount; j++)
        {
            if (spacialLookup[j].spatialHashKey != hashKey)
                break;

            unsigned int neighborIndex = spacialLookup[j].particleIndex;
            if (neighborIndex != particleIndex)
                callback(neighborIndex);
        }
    }
}