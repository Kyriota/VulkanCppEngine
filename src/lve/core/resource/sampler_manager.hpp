#pragma once

// lve
#include "lve/core/device.hpp"

// std
#include <unordered_map>

namespace lve
{
enum SamplerType
{
    DEFAULT,
};

class SamplerManager
{
public:
    SamplerManager(lve::Device &device) : lveDevice(device) {}
    ~SamplerManager();
    VkSampler getSampler(SamplerType type);

private:
    lve::Device &lveDevice;
    VkSampler getDefaultSampler();
    void createSampler(SamplerType type);
    void createSamplerWithInfo(VkSamplerCreateInfo &createInfo, VkSampler &sampler);
    std::unordered_map<SamplerType, VkSampler> samplers;
};
} // namespace lve