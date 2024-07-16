#pragma once

#include "lve/core/device.hpp"

// std
#include <stdexcept>
#include <string>

namespace lve
{
    void createShaderModule(Device &lveDevice, const std::vector<char> &code, VkShaderModule *shaderModule);

    void bind(VkCommandBuffer commandBuffer, VkPipeline pipeline);
} // namespace lve