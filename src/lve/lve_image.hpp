#pragma once

#include "lve_device.hpp"

// libs
#include <vulkan/vulkan.h>

// std
#include <unordered_map>

namespace lve
{
    class LveImage
    {
    public:
        LveImage(
            LveDevice &device,
            VkImageCreateInfo imageCreateInfo,
            VkMemoryPropertyFlags memPropertyFlags);

        ~LveImage();

        LveImage(const LveImage &) = delete;
        LveImage &operator=(const LveImage &) = delete;

        bool hasImageView(int id) const;

        void createImageView(
            int id,
            const VkImageViewCreateInfo *pImageViewCreateInfo);

        VkImageView getImageView(int id) const;

        void convertLayout(VkImageLayout newLayout);

    private:
        void allocateMemory(VkMemoryPropertyFlags memPropertyFlags);

        LveDevice &lveDevice;
        VkDeviceMemory imageMemory;
        VkImage image;
        std::unordered_map<int, VkImageView> imageViews;
    };

} // namespace lve
