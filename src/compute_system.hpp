#pragma once

#include "lve_device.hpp"
#include "lve_pipeline.hpp"

// std
#include <memory>

namespace lve
{
    class ComputeSystem
    {
    public:
        ComputeSystem(LveDevice &device);
        ~ComputeSystem();

        ComputeSystem(const ComputeSystem &) = delete;
        ComputeSystem &operator=(const ComputeSystem &) = delete;

        void dispatchComputePipeline(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height);

    private:
        void createComputePipelineLayout();
        void createComputePipeline();

        LveDevice &lveDevice;
        std::unique_ptr<LveComputePipeline> lveComputePipeline;
        VkPipelineLayout computePipelineLayout;
    };
} // namespace lve