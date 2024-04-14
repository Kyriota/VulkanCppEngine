#include "lve_image.hpp"

// std
#include <stdexcept>

namespace lve
{
    LveImage::LveImage(
        LveDevice &device,
        LveImageConfig imageConfig) : lveDevice{device}
    {
        createImage(imageConfig);
    }

    LveImage::~LveImage()
    {
        vkDestroyImageView(lveDevice.device(), imageView, nullptr);
        vkDestroyImage(lveDevice.device(), image, nullptr);
        vkFreeMemory(lveDevice.device(), imageMemory, nullptr);
    }

    void LveImage::createImage(
        LveImageConfig imageConfig)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = imageConfig.width;
        imageInfo.extent.height = imageConfig.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = imageConfig.format;
        imageInfo.tiling = imageConfig.tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = imageConfig.usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(lveDevice.device(), &imageInfo, nullptr, &image) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(lveDevice.device(), image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = lveDevice.findMemoryType(memRequirements.memoryTypeBits, imageConfig.properties);

        if (vkAllocateMemory(lveDevice.device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(lveDevice.device(), image, imageMemory, 0);

        imageView = createImageView(imageConfig.format);
    }

    VkImageView LveImage::createImageView(VkFormat format)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(lveDevice.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;
    }
} // namespace lve