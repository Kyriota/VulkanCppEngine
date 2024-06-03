#pragma once

#include "lve/core/device.hpp"

// std
#include <string>
#include <stdexcept>

namespace lve
{
    void createShaderModule(Device &lveDevice, const std::vector<char> &code, VkShaderModule *shaderModule);

    void bind(VkCommandBuffer commandBuffer, VkPipeline pipeline);
} // namespace lve