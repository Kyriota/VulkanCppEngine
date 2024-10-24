#pragma once

// lve
#include "lve/core/device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace lve
{

class DescriptorSetLayout
{
public:
    class Builder
    {
    public:
        Builder(Device &lveDevice) : lveDevice{lveDevice} {}

        Builder &addBinding(
            uint32_t binding,
            VkDescriptorType descriptorType,
            VkShaderStageFlags stageFlags,
            uint32_t count = 1);
        std::unique_ptr<DescriptorSetLayout> build() const;

    private:
        Device &lveDevice;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
    };

    DescriptorSetLayout(
        Device &lveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
    ~DescriptorSetLayout();
    DescriptorSetLayout(const DescriptorSetLayout &) = delete;
    DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

    VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

private:
    Device &lveDevice;
    VkDescriptorSetLayout descriptorSetLayout;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

    friend class DescriptorWriter;
};

class DescriptorPool
{
public:
    class Builder
    {
    public:
        Builder(Device &lveDevice) : lveDevice{lveDevice} {}

        Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
        Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
        Builder &setMaxSets(uint32_t count);
        std::unique_ptr<DescriptorPool> build() const;

    private:
        Device &lveDevice;
        std::vector<VkDescriptorPoolSize> poolSizes{};
        uint32_t maxSets = 1024;
        VkDescriptorPoolCreateFlags poolFlags = 0;
    };

    DescriptorPool(
        Device &lveDevice,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize> &poolSizes);
    ~DescriptorPool();
    DescriptorPool(const DescriptorPool &) = delete;
    DescriptorPool &operator=(const DescriptorPool &) = delete;

    bool allocateDescriptorSet(
        const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptorSet) const;

    void freeDescriptorSet(std::vector<VkDescriptorSet> &descriptorSet) const;

    void resetPool();

private:
    Device &lveDevice;
    VkDescriptorPool descriptorPool;

    friend class DescriptorWriter;
};

class DescriptorWriter
{
public:
    DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool);

    DescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
    DescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

    bool allocateDescriptorSet(VkDescriptorSet &set) const
    {
        return pool.allocateDescriptorSet(setLayout.getDescriptorSetLayout(), set);
    }
    void overwrite(VkDescriptorSet &set);

private:
    DescriptorSetLayout &setLayout;
    DescriptorPool &pool;
    std::vector<VkWriteDescriptorSet> writes;
};

} // namespace lve
