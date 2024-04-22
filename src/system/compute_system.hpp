#pragma once

#include "../lve/lve_device.hpp"
#include "../lve/lve_pipeline_compute.hpp"
#include "../lve/lve_frame_info.hpp"

// std
#include <memory>
#include <string>
#include <vector>

namespace lve
{
    class ComputeSystem
    {
    public:
        ComputeSystem(
            LveDevice &device,
            std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
            const std::string &compFilepath);
        ~ComputeSystem();

        ComputeSystem(const ComputeSystem &) = delete;
        ComputeSystem &operator=(const ComputeSystem &) = delete;

        void dispatchComputePipeline(FrameInfo frameInfo, uint32_t width, uint32_t height);

    private:
        void createComputePipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
        void createComputePipeline(const std::string &compFilepath);

        LveDevice &lveDevice;
        std::unique_ptr<LveComputePipeline> lveComputePipeline;
        VkPipelineLayout computePipelineLayout;
    };
} // namespace lve