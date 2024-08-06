#include "lve/core/resource/sampler_manager.hpp"

#include "lve/util/math.hpp"

namespace lve
{
    VkSampler SamplerManager::getDefaultSampler()
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
        createSamplerWithInfo(defaultSamplerInfo, sampler);

        return sampler;
    }

    void SamplerManager::createSamplerWithInfo(VkSamplerCreateInfo &createInfo, VkSampler &sampler)
    {
        if (vkCreateSampler(lveDevice.vkDevice(), &createInfo, nullptr, &sampler) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create texture sampler");
        }
    }

    void SamplerManager::createSampler(SamplerType type)
    {
        // use switch statement to create different types of samplers
        switch (type)
        {
        case SamplerType::DEFAULT:
            samplers[type] = getDefaultSampler();
            break;
        default:
            throw std::runtime_error("Failed to create sampler: unknown sampler type");
        }
    }

    VkSampler SamplerManager::getSampler(SamplerType type)
    {
        if (samplers.find(type) == samplers.end())
        {
            createSampler(type);
        }
        return samplers[type];
    }

    SamplerManager::~SamplerManager()
    {
        for (auto &sampler : samplers)
        {
            vkDestroySampler(lveDevice.vkDevice(), sampler.second, nullptr);
        }
    }
} // namespace lve