#pragma once

#include "lve/core/device.hpp"

// libs
#include <vulkan/vulkan.h>

// std
#include <unordered_map>

namespace lve
{
    class Image // wrapper for VkImage, VkDeviceMemory, and VkImageView
    {
    public:
        Image(Device &device, VkImageCreateInfo imageCreateInfo, VkMemoryPropertyFlags memPropertyFlags);
        Image(Device &device) : lveDevice(device) {}

        ~Image();

        Image(const Image &) = delete;
        Image &operator=(const Image &) = delete;

        Image(Image &&other);
        Image &operator=(Image &&other);

        bool hasImageView(int id) const;

        void createImageView(int id, const VkImageViewCreateInfo *pImageViewCreateInfo);

        VkImageView getImageView(int id) const;

        void convertLayout(VkImageLayout newLayout);

        VkImage getImage() const { return image; }

        VkExtent3D getExtent() const { return extent; }

        VkDescriptorImageInfo getDescriptorImageInfo(int imageViewId, VkSampler sampler) const;

    private:
        void allocateMemory(VkMemoryPropertyFlags memPropertyFlags);

        void cleanUp();

        Device &lveDevice;
        VkDeviceMemory imageMemory;
        VkImage image;
        VkExtent3D extent;
        std::unordered_map<int, VkImageView> imageViews;
        VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        bool initialized = false;
    };
} // namespace lve
