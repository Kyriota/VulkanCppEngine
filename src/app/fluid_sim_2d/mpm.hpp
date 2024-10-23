#pragma once

// libs
#include "include/glm.hpp"

// std
#include <cstddef>
#include <vector>

namespace app::fluidsim
{
class MPM
{
public:
    MPM();
    void substep(float deltaTime);

public: // getters
    size_t getParticleCount() const { return particleCount; }
    const std::vector<glm::vec2> &getPositionData() const { return x; }
    const std::vector<glm::vec2> &getVelocityData() const { return v; }

private: // structs

private:
    size_t particleCount = 4096;
    size_t gridCount = 128;
    float dx = 1.0f / gridCount;

    float particleRho = 1.0f;
    float particleVol = dx * dx * 0.25f;
    float particleMass = particleRho * particleVol;

    float gravity = 9.8f;
    int bound = 3;
    float E = 400.0f; // Young's modulus

    std::vector<glm::vec2> x; // position
    std::vector<glm::vec2> v; // velocity
    std::vector<glm::mat2> c; // deformation gradient velocity
    std::vector<float> j; // volume change (Jacobian)

    std::vector<std::vector<glm::vec2>> gridVel;
    std::vector<std::vector<float>> gridMass;
};
} // namespace app::fluidsim