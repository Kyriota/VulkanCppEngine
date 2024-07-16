#pragma once

#include "lve/core/device.hpp"
#include "lve/core/pipeline/pipeline.hpp"

// std
#include <string>
#include <vector>

namespace lve
{
    class ComputePipeline : public Pipeline
    {
    public:
        ComputePipeline(Device &device) : Pipeline(device) {}
        ComputePipeline(
            Device &device,
            const std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
            const std::string &compFilePath
        );
        ~ComputePipeline() { cleanUp(); }

        ComputePipeline(const ComputePipeline &) = delete;
        ComputePipeline &operator=(const ComputePipeline &) = delete;

        ComputePipeline(ComputePipeline &&other) noexcept;
        ComputePipeline &operator=(ComputePipeline &&other);

        void dispatchComputePipeline(
            VkCommandBuffer cmdBuffer,
            const VkDescriptorSet *pGlobalDescriptorSet,
            uint32_t width,
            uint32_t height
        );

    private:
        void release() override;

        void createPipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
        void createPipeline(const std::string &compFilePath);

        VkShaderModule compShaderModule;
    };
} // namespace lve
