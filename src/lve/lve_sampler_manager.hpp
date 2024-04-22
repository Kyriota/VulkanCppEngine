#pragma once

#include "lve_device.hpp"

// std
#include <unordered_map>

namespace lve
{
    enum class SamplerType
    {
        DEFAULT,
    };

    struct SamplerKey
    {
        SamplerType type;
        VkDevice device;

        bool operator==(const SamplerKey &other) const
        {
            return type == other.type && device == other.device;
        }
    };

    class LveSamplerManager // Static class to manage VkSampler objects
    {
    public:
        static VkSampler getSampler(SamplerKey key);
        static void clearSamplers();

    private:
        LveSamplerManager() = delete;

        static VkSampler getDefaultSampler(VkDevice device);
        static void createSamplerWithKey(SamplerKey key);
        static void createSamplerWithInfo(VkDevice device, VkSamplerCreateInfo &createInfo, VkSampler &sampler);
        static std::unordered_map<SamplerKey, VkSampler> samplers;
    };
} // namespace lve