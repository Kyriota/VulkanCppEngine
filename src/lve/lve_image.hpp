#pragma once

#include "lve_device.hpp"

// libs
#include <vulkan/vulkan.h>

namespace lve
{

    VkSampler createDefaultSampler(LveDevice &device);

    struct LveImageConfig
    {
        uint32_t width;
        uint32_t height;
        VkFormat format;
        VkImageTiling tiling;
        VkImageUsageFlags usage;
        VkMemoryPropertyFlags properties;
        VkImageLayout imgLayout;
        VkSampler sampler;
        VkImageLayout layout;
    };

    class LveImage
    {
    public:
        LveImage(
            LveDevice &device,
            LveImageConfig imageConfig);

        ~LveImage();

        VkImage getVkImage() const { return image; }
        VkImageView getVkImageView() const { return imageView; }
        VkDeviceMemory getVkDeviceMemory() const { return imageMemory; }
        VkDescriptorImageInfo *getDescriptorInfoPtr() { return &imageDescriptorInfo; }

    private:
        LveDevice &lveDevice;
        VkImage image;
        VkDeviceMemory imageMemory;
        VkImageView imageView;

        void createImage(LveImageConfig imageConfig);

        VkImageView createImageView(VkFormat format);

        VkDescriptorImageInfo imageDescriptorInfo;
    };

} // namespace lve
