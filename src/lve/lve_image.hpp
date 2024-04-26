#pragma once

#include "lve_device.hpp"

// libs
#include <vulkan/vulkan.h>

// std
#include <unordered_map>

namespace lve
{
    class LveImage // wrapper for VkImage, VkDeviceMemory, and VkImageView
    {
    public:
        LveImage(
            LveDevice &device,
            VkImageCreateInfo imageCreateInfo,
            VkMemoryPropertyFlags memPropertyFlags);
        LveImage(LveDevice &device) : lveDevice(device) {}

        ~LveImage();

        LveImage(const LveImage &) = delete;
        LveImage &operator=(const LveImage &) = delete;

        LveImage(LveImage&& other) noexcept;
        LveImage& operator=(LveImage&& other);

        bool hasImageView(int id) const;

        void createImageView(
            int id,
            const VkImageViewCreateInfo *pImageViewCreateInfo);

        VkImageView getImageView(int id) const;

        void convertLayout(VkImageLayout newLayout);

        VkImage getImage() const { return image; }

        VkExtent3D getExtent() const { return extent; }

        VkDescriptorImageInfo getDescriptorImageInfo(int imageViewId, VkSampler sampler) const;

    private:
        void allocateMemory(VkMemoryPropertyFlags memPropertyFlags);

        void cleanUp();

        LveDevice &lveDevice;
        VkDeviceMemory imageMemory;
        VkImage image;
        VkExtent3D extent;
        std::unordered_map<int, VkImageView> imageViews;
        VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        bool initialized = false;
    };
} // namespace lve
