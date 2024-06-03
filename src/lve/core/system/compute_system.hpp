#pragma once

#include "lve/core/device.hpp"
#include "lve/core/pipeline/compute_pipeline.hpp"

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
            Device &device,
            std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
            const std::string &compFilepath);
        ComputeSystem(Device &device) : lveDevice(device) {}
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

        Device &lveDevice;
        std::unique_ptr<ComputePipeline> lveComputePipeline;
        VkPipelineLayout computePipelineLayout;

        bool initialized = false;
    };
} // namespace lve