#pragma once

#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_window.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

namespace lve
{
    class LveRenderer
    {
    public:
        LveRenderer(LveWindow &window, LveDevice &device);
        ~LveRenderer();

        LveRenderer(const LveRenderer &) = delete;
        LveRenderer &operator=(const LveRenderer &) = delete;

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

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

        using SwapChainResizedCallback = std::function<void(VkExtent2D)>;
        void registerSwapChainResizedCallback(const std::string &name, SwapChainResizedCallback callback) { swapChainResizedCallbacks[name] = callback; }
        void unregisterSwapChainResizedCallback(const std::string &name) { swapChainResizedCallbacks.erase(name); }

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        bool recreateSwapChain();

        LveWindow &lveWindow;
        LveDevice &lveDevice;
        std::unique_ptr<LveSwapChain> lveSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted{false};

        std::unordered_map<std::string, SwapChainResizedCallback> swapChainResizedCallbacks;
    };
} // namespace lve
