#pragma once

// std
#include <chrono>
#include <functional>

namespace lve
{
class FpsManager
{
public:
    FpsManager(int minFps, int maxFps = 0); // 0 means no limit

    void renderStart() { lastFrameStartTime = std::chrono::high_resolution_clock::now(); }
    double step(std::function<void(int)> callback = [](int) {}); // returns frame duration
    void fpsLimitBusyWait();

    double getMinFrameDuration() const { return minFrameDuration; }
    double getMaxFrameDuration() const { return maxFrameDuration; }

private:
    int frameCount = 0;
    std::chrono::_V2::system_clock::time_point countStartTime, currentFrameStartTime,
        lastFrameStartTime;
    double maxFrameDuration;
    double minFrameDuration;
    bool isFrameRateLimited = false;
};
} // namespace lve