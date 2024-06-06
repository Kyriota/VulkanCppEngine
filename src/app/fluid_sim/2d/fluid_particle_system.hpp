#pragma once

#include "lve/go/geo/line.hpp"
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

    void updateWindowExtent(VkExtent2D newExtent);
    void updateParticleData(float deltaTime);

    unsigned int getParticleCount() const { return particleCount; }
    float getSmoothRadius() const { return smoothRadius; }
    float getTargetDensity() const { return targetDensity; }
    float getDataScale() const { return dataScale; }
    std::vector<glm::vec2> &getPositionData() { return positionData; }
    std::vector<glm::vec2> &getVelocityData() { return velocityData; }

    void setRangeForcePos(bool sign, glm::vec2 mousePosition);

    // control and debug
    void togglePause() { isPaused = !isPaused; }
    void printDensity(glm::vec2 mousePosition);
    void printPressureForce(glm::vec2 mousePosition);
    std::vector<int> &getFirstParticleNeighborIndex() { return firstParticleNeighborIndex; }
    std::vector<lve::Line> &getDebugLines() { return debugLines; }

private:
    struct SpatialHashEntry
    {
        unsigned int particleIndex;
        unsigned int spatialHashKey;
    };

    std::string configFilePath;
    unsigned int particleCount;
    VkExtent2D windowExtent;
    glm::vec2 scaledWindowExtent;

    // control and debug
    std::vector<int> firstParticleNeighborIndex;
    std::vector<lve::Line> debugLines;
    bool isPaused = false;
    unsigned int getClosetParticleIndex(glm::vec2 mousePosition);
    std::vector<glm::vec2> pressureForceData;
    std::vector<glm::vec2> externalForceData;
    std::vector<glm::vec2> viscosityForceData;

    // simulation parameters
    float smoothRadius;
    float boundaryMultipler;
    float targetDensity;
    float pressureMultiplier;
    float nearPressureMultiplier;
    float viscosityMultiplier;
    float gravityAccValue;
    float dataScale;
    float rangeForceScale;
    float rangeForceRadius;
    float lookAheadTime = 1.0 / 120.0;
    float maxDeltaTime = 1.0 / 120.0;
    float boundaryMargin = 0.5;

    // particle data
    std::vector<glm::vec2> positionData;
    std::vector<glm::vec2> nextPositionData;
    std::vector<glm::vec2> velocityData;
    std::vector<float> densityData;
    std::vector<float> massData;
    void initParticleData(glm::vec2 startPoint, float stride, float maxWidth, bool randomize);
    void initSimParams(lve::io::YamlConfig &config);
    glm::vec2 scaledPos2ScreenPos(glm::vec2 scaledPos) const;

    // kernels
    float kernelPoly6_2D(float distance, float radius) const;
    float scalingFactorPoly6_2D;
    float scalingFactorPoly6_2D_atZero;
    float kernelSpikyPow3_2D(float distance, float radius) const;
    float derivativeSpikyPow3_2D(float distance, float radius) const;
    float scalingFactorSpikyPow3_2D;
    float scalingFactorSpikyPow3_2D_atZero;
    float kernelSpikyPow2_2D(float distance, float radius) const;
    float derivativeSpikyPow2_2D(float distance, float radius) const;
    float scalingFactorSpikyPow2_2D;
    float scalingFactorSpikyPow2_2D_atZero;

    // update rules
    float calculateDensity(unsigned int particleIndex);
    glm::vec2 calculatePressureForce(unsigned int particleIndex);
    glm::vec2 calculateExternalForce(unsigned int particleIndex);
    glm::vec2 calculateViscosityForce(unsigned int particleIndex);
    glm::vec2 calculateNearPressureForce(unsigned int particleIndex);

    // hash grid
    std::vector<SpatialHashEntry> spacialLookup;
    std::vector<int> spacialLookupEntry;
    glm::int2 pos2gridCoord(glm::vec2 position, float gridWidth) const;
    int hashGridCoord2D(glm::int2 gridCoord) const;
    void updateSpatialLookup();
    void foreachNeighbor(unsigned int particleIndex, std::function<void(int)> callback);
    const glm::int2 offset2D[9] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {0, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};

    // external force
    struct RangeForceInfo
    {
        bool active;
        bool sign;
        glm::vec2 position;
    };
    RangeForceInfo rangeForceInfo = {false, false, glm::vec2(0.0f, 0.0f)};
};