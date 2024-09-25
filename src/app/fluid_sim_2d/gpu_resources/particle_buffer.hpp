#pragma once

#include "app/fluid_sim_2d/fluid_particle_system.hpp"
#include "lve/core/frame_manager.hpp"

namespace app::fluidsim2d
{
class ParticleBuffers
{
public: // constructors
    ParticleBuffers(lve::FrameManager &frameManager, FluidParticleSystem &fluidParticleSys);
    ParticleBuffers(const ParticleBuffers &) = delete;
    ParticleBuffers &operator=(const ParticleBuffers &) = delete;

public: // methods
    void writeParticleBuffer();

public: // getters
    lve::Buffer &getParticleBuffer() { return *particleBuffer; }
    lve::Buffer &getNeighborBuffer() { return *neighborBuffer; }

private: // methods
    void initParticleBuffer();

private: // variables
    lve::FrameManager &lveFrameManager;
    FluidParticleSystem &fluidParticleSys;

    // buffers
    std::unique_ptr<lve::Buffer> particleBuffer;
    std::unique_ptr<lve::Buffer> neighborBuffer;
};
} // namespace app::fluidsim2d