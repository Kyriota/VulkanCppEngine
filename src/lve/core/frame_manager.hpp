#pragma once

#include "lve/core/device.hpp"
#include "lve/core/swap_chain.hpp"
#include "lve/core/window.hpp"

// std
#include <cassert>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace lve
{
    class FrameManager
    {
    public:
        FrameManager(Window &window, Device &device);
        ~FrameManager();

        FrameManager(const FrameManager &) = delete;
        FrameManager &operator=(const FrameManager &) = delete;

        VkRenderPass getSwapChainRenderPass() const { return lveSwapChain->getRenderPass(); }
        float getAspectRatio() const { return lveSwapChain->extentAspectRatio(); }
        bool isFrameInProgress() const { return isFrameStarted; }

        VkCommandBuffer getCurrentCommandBuffer() const
        {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const
        {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        Device &getDevice() const { return lveDevice; }
        Window &getWindow() const { return lveWindow; }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

        using SwapChainResizedCallback = std::function<void(VkExtent2D)>;
        void registerSwapChainResizedCallback(const std::string &name, SwapChainResizedCallback callback)
        {
            swapChainResizedCallbacks[name] = callback;
        }
        void unregisterSwapChainResizedCallback(const std::string &name) { swapChainResizedCallbacks.erase(name); }

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        bool recreateSwapChain();

        Window &lveWindow;
        Device &lveDevice;
        std::unique_ptr<SwapChain> lveSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted{false};

        std::unordered_map<std::string, SwapChainResizedCallback> swapChainResizedCallbacks;
    };
} // namespace lve
