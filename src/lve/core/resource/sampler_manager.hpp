#pragma once

#include "lve/core/device.hpp"

// std
#include <unordered_map>

namespace lve
{
class Samplers // Singleton
{
public: // enums
    enum class Type
    {
        DEFAULT,
    };

public: // methods
    static VkSampler getSampler(lve::Device &lveDevice, Type type);

private: // structs
    struct Key
    {
        Type type;
        lve::Device *lveDevice;

        bool operator==(const Key &other) const
        {
            return type == other.type && lveDevice->vkDevice() == other.lveDevice->vkDevice();
        }

        struct Hash
        {
            std::size_t operator()(const Key &key) const
            {
                return std::hash<int>()(static_cast<int>(key.type)) ^
                       std::hash<VkDevice>()(key.lveDevice->vkDevice());
            }
        };
    };

private: // constructors
    Samplers() = default;
    ~Samplers();

private: // methods
    static VkSampler getDefaultSampler(lve::Device &lveDevice);
    static void createSampler(Key key);
    static void
    createSamplerWithInfo(lve::Device &lveDevice, VkSamplerCreateInfo &createInfo, VkSampler &sampler);

private: // variables
    std::unordered_map<Key, VkSampler, Key::Hash> samplerMap;
};
} // namespace lve