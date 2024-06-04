#pragma once

#define M_PI 3.14159265358979323846

#include "lve/util/math.hpp"
#include "lve/util/file_io.hpp"

// libs
#include "include/glm.hpp"
#include <vulkan/vulkan.h>

// std
#include <string>
#include <vector>

class FluidParticleSystem
{
public:
    FluidParticleSystem(const std::string &configFilePath, VkExtent2D windowExtent);

    void reloadConfigParam();

    void updateParticleData(float deltaTime);
    void updateWindowExtent(VkExtent2D newExtent) { windowExtent = newExtent; }

    float getParticleCount() const { return particleCount; }
    float getSmoothRadius() const { return smoothRadius; }
    float getTargetDensity() const { return targetDensity; }
    float getDataScale() const { return dataScale; }
    std::vector<glm::vec2> &getPositionData() { return positionData; }
    std::vector<glm::vec2> &getVelocityData() { return velocityData; }

    void setExternalForcePos(bool sign, glm::vec2 position);

private:
    struct SpatialHashEntry
    {
        unsigned int particleIndex;
        unsigned int spatialHashKey;
    };

    std::string configFilePath;
    unsigned int particleCount;
    VkExtent2D windowExtent;

    // simulation parameters
    float smoothRadius;
    float collisionDamping;
    float targetDensity;
    float pressureMultiplier;
    float gravityAccValue;
    float dataScale;
    float externalForceScale;
    float externalForceRadius;
    float lookAheadTime = 1.0 / 120.0;

    // particle data
    std::vector<glm::vec2> positionData;
    std::vector<glm::vec2> nextPositionData;
    std::vector<glm::vec2> velocityData;
    std::vector<float> densityData;
    std::vector<float> massData;
    void initParticleData(glm::vec2 startPoint, float stride, float maxWidth, bool randomize);
    void initSimParams(lve::io::YamlConfig &config);

    // kernels
    float kernelPoly6_2D(float distance, float radius) const;
    float scalingFactorPoly6_2D;
    float kernelSpikyPow3_2D(float distance, float radius) const;
    float derivativeSpikyPow3_2D(float distance, float radius) const;
    float scalingFactorSpikyPow3_2D;
    float kernelSpikyPow2_2D(float distance, float radius) const;
    float derivativeSpikyPow2_2D(float distance, float radius) const;
    float scalingFactorSpikyPow2_2D;

    // update rules
    float calculateDensity(unsigned int particleIndex);
    glm::vec2 calculatePressureForce(unsigned int particleIndex);
    glm::vec2 calculateExternalForce(unsigned int particleIndex);
    void handleBoundaryCollision();

    // hash grid
    std::vector<SpatialHashEntry> spacialLookup;
    std::vector<int> spacialLookupEntry;
    glm::int2 pos2gridCoord(glm::vec2 position, int gridWidth) const;
    int hashGridCoord2D(glm::int2 gridCoord) const;
    void updateSpatialLookup();
    void foreachNeighbor(unsigned int particleIndex, std::function<void(int)> callback);
    const glm::int2 offset2D[9] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {0, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};

    // external force
    struct ExternalForceInfo
    {
        bool active;
        bool sign;
        glm::vec2 position;
    };
    ExternalForceInfo externalForceInfo = {false, false, glm::vec2(0.0f, 0.0f)};
};