#pragma once

#include "lve_device.hpp"

// libs
#include "stb_image.h"
#include <vulkan/vulkan.h>

namespace lve
{

    struct LveImageConfig
    {
        uint32_t width;
        uint32_t height;
        VkFormat format;
        VkImageTiling tiling;
        VkImageUsageFlags usage;
        VkMemoryPropertyFlags properties;
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

    private:
        LveDevice &lveDevice;
        VkImage image;
        VkDeviceMemory imageMemory;
        VkImageView imageView;

        void createImage(LveImageConfig imageConfig);

        VkImageView createImageView(VkFormat format);
    };

} // namespace lve
