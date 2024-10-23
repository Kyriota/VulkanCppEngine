#include "mpm.hpp"

// lve
#include "lve/util/math.hpp"

// std
#include <array>
#include <omp.h>
#include <random>

namespace app::fluidsim
{
MPM::MPM()
{
    x.resize(particleCount);
    v.resize(particleCount);
    c.resize(particleCount);
    j.resize(particleCount);

    gridVel.resize(gridCount);
    gridMass.resize(gridCount);
    for (size_t i = 0; i < gridCount; i++)
    {
        gridVel[i].resize(gridCount);
        gridMass[i].resize(gridCount);
    }

    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> dis(0.2f, 0.6f);
    for (size_t i = 0; i < particleCount; i++)
    {
        x[i] = glm::vec2(dis(gen), dis(gen));
        v[i] = glm::vec2(0.0f, 0.0f);
        j[i] = 1.0f;
    }
}

void MPM::substep(float deltaTime)
{
    // clear grid
    for (size_t i = 0; i < gridCount; i++)
    {
        for (size_t j = 0; j < gridCount; j++)
        {
            gridVel[i][j] = glm::vec2(0.0f, 0.0f);
            gridMass[i][j] = 0.0f;
        }
    }
    // particle to grid
    for (size_t p = 0; p < particleCount; p++)
    {
        glm::vec2 gridPos = x[p] / dx;
        glm::vec2 baseCoord = glm::floor(gridPos - glm::vec2(0.5f, 0.5f));
        glm::vec2 localPos = gridPos - glm::vec2(baseCoord);
        std::array<glm::vec2, 3> w = {
            0.5f * lve::math::square(1.5f - localPos),
            0.75f - lve::math::square(localPos - 1.0f),
            0.5f * lve::math::square(localPos - 0.5f)};
        float stress = -deltaTime * 4.0f * E * particleVol * (j[p] - 1.0f) * gridCount * gridCount;
        glm::mat2 affine = glm::mat2(stress, 0.0f, 0.0f, stress) + particleMass * c[p];
        for (size_t i = 0; i < 3; i++)
        {
            for (size_t j = 0; j < 3; j++)
            {
                glm::vec2 offset = glm::vec2(i, j);
                glm::vec2 dpos = (offset - localPos) * dx;
                float weight = w[i].x * w[j].y;
                gridVel[baseCoord.x + i][baseCoord.y + j] +=
                    weight * (particleMass * v[p] + affine * dpos);
                gridMass[baseCoord.x + i][baseCoord.y + j] += weight * particleMass;
            }
        }
    }
    // grid boundary and gravity
    for (size_t i = 0; i < gridCount; i++)
    {
        for (size_t j = 0; j < gridCount; j++)
        {
            if (gridMass[i][j] > 0.0f)
                gridVel[i][j] /= gridMass[i][j];

            gridVel[i][j].y += gravity * deltaTime;

            if (i < bound && gridVel[i][j].x < 0.0f)
                gridVel[i][j].x = 0.0f;
            else if (i > gridCount - bound && gridVel[i][j].x > 0.0f)
                gridVel[i][j].x = 0.0f;
            if (j < bound && gridVel[i][j].y < 0.0f)
                gridVel[i][j].y = 0.0f;
            else if (j > gridCount - bound && gridVel[i][j].y > 0.0f)
                gridVel[i][j].y = 0.0f;
        }
    }
#pragma omp parallel for
    // grid to particle
    for (int p = 0; p < particleCount; p++)
    {
        glm::vec2 gridPos = x[p] / dx;
        glm::vec2 baseCoord = glm::floor(gridPos - glm::vec2(0.5f, 0.5f));
        glm::vec2 localPos = gridPos - glm::vec2(baseCoord);
        std::array<glm::vec2, 3> w = {
            0.5f * lve::math::square(1.5f - localPos),
            0.75f - lve::math::square(localPos - 1.0f),
            0.5f * lve::math::square(localPos - 0.5f)};
        glm::vec2 newV = glm::vec2(0.0f, 0.0f);
        glm::mat2 newC = glm::mat2(0.0f);
        for (size_t i = 0; i < 3; i++)
        {
            for (size_t j = 0; j < 3; j++)
            {
                glm::vec2 offset = glm::vec2(i, j);
                glm::vec2 dpos = (offset - localPos) * dx;
                float weight = w[i].x * w[j].y;
                glm::vec2 gridVelValue = gridVel[baseCoord.x + i][baseCoord.y + j];
                newV += weight * gridVelValue;
                newC +=
                    4.0f * weight * gridCount * gridCount * glm::outerProduct(gridVelValue, dpos);
            }
        }
        v[p] = newV;
        x[p] += deltaTime * v[p];
        c[p] = newC;
        j[p] *= 1.0f + deltaTime * lve::math::trace(c[p]);
    }
    float dxMulBound = dx * bound;
}
} // namespace app::fluidsim