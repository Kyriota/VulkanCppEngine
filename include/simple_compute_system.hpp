#pragma once

#include "lve_device.hpp"
#include "lve_pipeline.hpp"

// std
#include <memory>

namespace lve
{
    class SimpleComputeSystem
    {
    public:
        SimpleComputeSystem(LveDevice &device);
        ~SimpleComputeSystem();

        SimpleComputeSystem(const SimpleComputeSystem &) = delete;
        SimpleComputeSystem &operator=(const SimpleComputeSystem &) = delete;

        void dispatchComputePipeline(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height);

    private:
        void createComputePipelineLayout();
        void createComputePipeline();

        LveDevice &lveDevice;
        std::unique_ptr<LveComputePipeline> lveComputePipeline;
        VkPipelineLayout computePipelineLayout;
    };
} // namespace lve