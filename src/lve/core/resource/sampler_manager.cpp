#include "lve/core/resource/sampler_manager.hpp"

#include "lve/util/math.hpp"

namespace lve
{
VkSampler Samplers::getDefaultSampler(lve::Device &lveDevice)
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
    createSamplerWithInfo(lveDevice, defaultSamplerInfo, sampler);

    return sampler;
}

void Samplers::createSamplerWithInfo(
    lve::Device &lveDevice,
    VkSamplerCreateInfo &createInfo,
    VkSampler &sampler
)
{
    if (vkCreateSampler(lveDevice.vkDevice(), &createInfo, nullptr, &sampler) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture sampler");
    }
}

void Samplers::createSampler(Key key)
{
    static Samplers instance;
    // use switch statement to create different types of samplers
    switch (key.type)
    {
    case Type::DEFAULT:
        instance.samplerMap[key] = getDefaultSampler(*key.lveDevice);
        break;
    default:
        throw std::runtime_error("Failed to create sampler: unknown sampler type");
    }
}

VkSampler Samplers::getSampler(lve::Device &lveDevice, Type type)
{
    static Samplers instance;
    Key key{type, &lveDevice};
    if (instance.samplerMap.find(key) == instance.samplerMap.end())
    {
        createSampler(key);
    }
    return instance.samplerMap[key];
}

Samplers::~Samplers()
{
    for (auto &sampler : samplerMap)
    {
        vkDestroySampler(sampler.first.lveDevice->vkDevice(), sampler.second, nullptr);
    }
}
} // namespace lve