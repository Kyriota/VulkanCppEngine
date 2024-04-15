#pragma once

#include "../lve/lve_device.hpp"
#include "../lve/lve_pipeline.hpp"
#include "../lve/lve_frame_info.hpp"

// std
#include <memory>
#include <string>

namespace lve
{
    class ComputeSystem
    {
    public:
        ComputeSystem(LveDevice &device, const std::string &compFilepath);
        ~ComputeSystem();

        ComputeSystem(const ComputeSystem &) = delete;
        ComputeSystem &operator=(const ComputeSystem &) = delete;

        void dispatchComputePipeline(FrameInfo frameInfo, uint32_t width, uint32_t height);

    private:
        void createComputePipelineLayout();
        void createComputePipeline(const std::string &compFilepath);

        LveDevice &lveDevice;
        std::unique_ptr<LveComputePipeline> lveComputePipeline;
        VkPipelineLayout computePipelineLayout;
    };
} // namespace lve