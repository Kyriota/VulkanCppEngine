#pragma once

#include "lve_device.hpp"

// std
#include <string>
#include <stdexcept>

namespace lve
{
    void createShaderModule(LveDevice &lveDevice, const std::vector<char> &code, VkShaderModule *shaderModule);

    void bind(VkCommandBuffer commandBuffer, VkPipeline pipeline);
} // namespace lve