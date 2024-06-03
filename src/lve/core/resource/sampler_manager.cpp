#include "lve/core/resource/sampler_manager.hpp"
#include "lve/util/math.hpp"

namespace std
{
    template <>
    struct hash<lve::SamplerKey>
    {
        size_t operator()(const lve::SamplerKey &key) const
        {
            size_t seed = 0;
            lve::math::hashCombine(seed, key.type, key.device);
            return seed;
        }
    };
} // namespace std

namespace lve
{
    std::unordered_map<SamplerKey, VkSampler> SamplerManager::samplers;

    VkSampler SamplerManager::getDefaultSampler(VkDevice device)
    {
        VkSamplerCreateInfo defaultSamplerInfo{};
        defaultSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        defaultSamplerInfo.magFilter = VK_FILTER_LINEAR;
        defaultSamplerInfo.minFilter = VK_FILTER_LINEAR;
        defaultSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        defaultSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        defaultSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        defaultSamplerInfo.anisotropyEnable = VK_TRUE;
        defaultSamplerInfo.maxAnisotropy = 16;
        defaultSamplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        defaultSamplerInfo.unnormalizedCoordinates = VK_FALSE;
        defaultSamplerInfo.compareEnable = VK_FALSE;
        defaultSamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        defaultSamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        defaultSamplerInfo.mipLodBias = 0.0f;
        defaultSamplerInfo.minLod = 0.0f;
        defaultSamplerInfo.maxLod = 0.0f;

        VkSampler sampler;
        createSamplerWithInfo(device, defaultSamplerInfo, sampler);

        return sampler;
    }

    void SamplerManager::createSamplerWithInfo(VkDevice device, VkSamplerCreateInfo &createInfo, VkSampler &sampler)
    {
        if (vkCreateSampler(device, &createInfo, nullptr, &sampler) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create texture sampler");
        }
    }

    void SamplerManager::createSamplerWithKey(SamplerKey key)
    {
        // use switch statement to create different types of samplers
        switch (key.type)
        {
        case SamplerType::DEFAULT:
            samplers[key] = getDefaultSampler(key.device);
            break;
        default:
            throw std::runtime_error("Failed to create sampler: unknown sampler type");
        }
    }

    VkSampler SamplerManager::getSampler(SamplerKey key)
    {
        if (samplers.find(key) == samplers.end())
        {
            createSamplerWithKey(key);
        }
        return samplers[key];
    }

    void SamplerManager::clearSamplers()
    {
        for (auto &sampler : samplers)
        {
            vkDestroySampler(sampler.first.device, sampler.second, nullptr);
        }
        samplers.clear();
    }
} // namespace lve