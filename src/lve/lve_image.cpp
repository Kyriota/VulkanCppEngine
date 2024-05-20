#include "lve_image.hpp"

// std
#include <stdexcept>

namespace lve
{
    LveImage::LveImage(
        LveDevice &device,
        VkImageCreateInfo imageCreateInfo,
        VkMemoryPropertyFlags memPropertyFlags)
        : lveDevice{device}
    {
        VkImageLayout initialLayout = imageCreateInfo.initialLayout;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        extent = imageCreateInfo.extent;

        if (vkCreateImage(lveDevice.device(), &imageCreateInfo, nullptr, &image) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image");
        }

        allocateMemory(memPropertyFlags);

        initialized = true;

        if (initialLayout != VK_IMAGE_LAYOUT_UNDEFINED)
        {
            convertLayout(initialLayout);
        }
    }

    LveImage::~LveImage()
    {
        cleanUp();
    }

    LveImage::LveImage(LveImage &&other) noexcept
        : lveDevice{other.lveDevice},
          imageMemory{other.imageMemory},
          image{other.image},
          extent{other.extent},
          imageViews{std::move(other.imageViews)},
          imageLayout{other.imageLayout},
          initialized{other.initialized}
    {
        other.image = nullptr;
        other.imageMemory = nullptr;
    }

    LveImage &LveImage::operator=(LveImage &&other)
    {
        if (this->lveDevice.device() != other.lveDevice.device())
        {
            throw std::runtime_error("Moved LveImage objects must be on the same LveDevice");
        }

        if (this != &other)
        {
            // Clean up existing resources
            cleanUp();

            imageMemory = other.imageMemory;
            image = other.image;
            imageViews = std::move(other.imageViews);
            imageLayout = other.imageLayout;
            extent = other.extent;
            initialized = other.initialized;

            // Reset other object
            other.image = nullptr;
            other.imageMemory = nullptr;
        }

        return *this;
    }

    bool LveImage::hasImageView(int id) const
    {
        return imageViews.find(id) != imageViews.end();
    }

    void LveImage::createImageView(
        int id,
        const VkImageViewCreateInfo *pImageViewCreateInfo)
    {
        if (!initialized)
        {
            throw std::runtime_error("Image must be initialized before creating image view");
        }

        if (hasImageView(id))
        {
            throw std::runtime_error("Image view already exists for id: " + std::to_string(id));
        }

        VkImageView imageView;
        if (vkCreateImageView(lveDevice.device(), pImageViewCreateInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create texture image view");
        }

        imageViews[id] = imageView;
    }

    VkImageView LveImage::getImageView(int id) const
    {
        if (!hasImageView(id))
        {
            throw std::runtime_error("Image view not found for id: " + std::to_string(id));
        }

        return imageViews.at(id);
    }

    void LveImage::convertLayout(VkImageLayout newLayout)
    {
        if (!initialized)
        {
            throw std::runtime_error("Image must be initialized before converting layout");
        }

        VkCommandBuffer commandBuffer = lveDevice.beginSingleTimeCommands();

        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = newLayout;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.levelCount = 1;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &imageMemoryBarrier);

        lveDevice.endSingleTimeCommands(commandBuffer);

        imageLayout = newLayout;
    }

    VkDescriptorImageInfo LveImage::getDescriptorImageInfo(int imageViewId, VkSampler sampler) const
    {
        return VkDescriptorImageInfo{
            .sampler = sampler,
            .imageView = getImageView(imageViewId),
            .imageLayout = imageLayout};
    }

    void LveImage::allocateMemory(VkMemoryPropertyFlags memPropertyFlags)
    {
        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(lveDevice.device(), image, &memoryRequirements);

        VkMemoryAllocateInfo memoryAllocateInfo{};
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = lveDevice.findMemoryType(
            memoryRequirements.memoryTypeBits,
            memPropertyFlags);

        if (vkAllocateMemory(lveDevice.device(), &memoryAllocateInfo, nullptr, &imageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate image memory");
        }

        vkBindImageMemory(lveDevice.device(), image, imageMemory, 0);
    }

    void LveImage::cleanUp()
    {
        if (!initialized)
        {
            return;
        }

        for (auto imageView : imageViews)
        {
            vkDestroyImageView(lveDevice.device(), imageView.second, nullptr);
        }
        vkDestroyImage(lveDevice.device(), image, nullptr);
        vkFreeMemory(lveDevice.device(), imageMemory, nullptr);
    }
} // namespace lve