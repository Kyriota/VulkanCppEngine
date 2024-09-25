#include "fps.hpp"

namespace lve
{
FpsManager::FpsManager(int minFps, int maxFps)
{
    minFrameDuration = 1.0 / minFps;
    maxFrameDuration = maxFps == 0 ? 0 : 1.0 / maxFps;
}

double FpsManager::step(std::function<void(int)> callback)
{
    frameCount++;
    currentFrameStartTime = std::chrono::high_resolution_clock::now();
    if (std::chrono::duration<double>(currentFrameStartTime - countStartTime).count() >= 1.0)
    {
        callback(frameCount);
        frameCount = 0;
        countStartTime = currentFrameStartTime;
    }
    double frameDuration =
        std::chrono::duration<double>(currentFrameStartTime - lastFrameStartTime).count();
    lastFrameStartTime = currentFrameStartTime;
    return frameDuration;
}

void FpsManager::fpsLimitBusyWait()
{
    if (isFrameRateLimited)
        while (std::chrono::duration<double>(
                   std::chrono::high_resolution_clock::now() - currentFrameStartTime)
                   .count() < minFrameDuration)
            continue;
}
} // namespace lve