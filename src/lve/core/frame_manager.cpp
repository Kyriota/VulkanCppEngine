#include "lve/core/frame_manager.hpp"

// std
#include <array>
#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace lve
{

FrameManager::FrameManager(Window &window, Device &device) : lveWindow{window}, lveDevice{device}
{
    recreateSwapChain();
    createCommandBuffers();
}

FrameManager::~FrameManager() { freeCommandBuffers(); }

bool FrameManager::recreateSwapChain()
{
    if (lveWindow.isWindowMinimized())
    {
        std::unique_lock<std::mutex> renderLock(lveWindow.renderMutex);
        lveWindow.renderCondVar.wait(renderLock, [this] {
            return !lveWindow.isWindowMinimized();
        });
        return false;
    }

    VkExtent2D windowExtent = lveWindow.getExtent();
    VkExtent2D swapChainExtent =
        lveSwapChain == nullptr ? VkExtent2D{0, 0} : lveSwapChain->getSwapChainExtent();

    vkDeviceWaitIdle(lveDevice.vkDevice());

    if (lveSwapChain == nullptr)
    {
        lveSwapChain = std::make_unique<SwapChain>(lveDevice, windowExtent);
    }
    else
    {
        printf(
            " >>> recreating swap chain with window extent: %d, %d\n",
            windowExtent.width,
            windowExtent.height);
        std::shared_ptr<SwapChain> oldSwapChain = std::move(lveSwapChain);
        lveSwapChain = std::make_unique<SwapChain>(lveDevice, windowExtent, oldSwapChain);

        if (!oldSwapChain->compareSwapFormats(*lveSwapChain.get()))
        {
            throw std::runtime_error("Swap chain image(or depth) format has changed!");
        }
    }

    return true;
}

void FrameManager::createCommandBuffers()
{
    commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = lveDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(lveDevice.vkDevice(), &allocInfo, commandBuffers.data()) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void FrameManager::freeCommandBuffers()
{
    vkFreeCommandBuffers(
        lveDevice.vkDevice(),
        lveDevice.getCommandPool(),
        static_cast<uint32_t>(commandBuffers.size()),
        commandBuffers.data());
    commandBuffers.clear();
}

VkCommandBuffer FrameManager::beginFrame()
{
    assert(!isFrameStarted && "Can't call beginFrame while already in progress");

    VkResult result = lveSwapChain->acquireNextImage(&currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return nullptr;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    isFrameStarted = true;

    VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    return commandBuffer;
}

void FrameManager::endFrame()
{
    assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
    VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkResult result = lveSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || // The swap chain has become
                                              // incompatible with the surface
                                              // and can no longer be used for
                                              // rendering. Usually happens
                                              // after a window resize.

        result == VK_SUBOPTIMAL_KHR // The swap chain can still be used to
                                    // successfully present to the surface,
                                    // but the surface properties are no longer
                                    // matched exactly.
    )
    {
        // This branch is usually entered when the window is resized or
        // minimized
        bool isSwapChainRecreated = recreateSwapChain();
        if (isSwapChainRecreated)
        {
            for (const auto &callback : swapChainResizedCallbacks)
            {
                callback.second(lveSwapChain->getSwapChainExtent());
            }
        }
    }

    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void FrameManager::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
    assert(
        commandBuffer == getCurrentCommandBuffer() &&
        "Can't begin render pass on command buffer from a different frame");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = lveSwapChain->getRenderPass();
    renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(currentImageIndex);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{
        {0, 0},
        lveSwapChain->getSwapChainExtent()
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void FrameManager::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
    assert(
        commandBuffer == getCurrentCommandBuffer() &&
        "Can't end render pass on command buffer from a different frame");
    vkCmdEndRenderPass(commandBuffer);
}
} // namespace lve
