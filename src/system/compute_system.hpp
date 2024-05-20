#pragma once

#include "lve/lve_device.hpp"
#include "lve/lve_pipeline_compute.hpp"

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
        ComputeSystem(LveDevice &device) : lveDevice(device) {}
        ~ComputeSystem();

        ComputeSystem(const ComputeSystem &) = delete;
        ComputeSystem &operator=(const ComputeSystem &) = delete;

        ComputeSystem(ComputeSystem &&other) noexcept;
        ComputeSystem &operator=(ComputeSystem &&other);

        void dispatchComputePipeline(
            VkCommandBuffer cmdBuffer,
            const VkDescriptorSet *pGlobalDescriptorSet,
            uint32_t width, uint32_t height);

    private:
        void cleanUp();

        void createComputePipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
        void createComputePipeline(const std::string &compFilepath);

        LveDevice &lveDevice;
        std::unique_ptr<LveComputePipeline> lveComputePipeline;
        VkPipelineLayout computePipelineLayout;

        bool initialized = false;
    };
} // namespace lve