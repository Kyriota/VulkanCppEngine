#pragma once

// lve
#include "lve/core/device.hpp"
#include "lve/core/resource/buffer.hpp"

// libs
#include "include/glm.hpp"

// std
#include <vector>

namespace lve
{
struct Point
{
    Point() = default;

    Point(glm::vec3 position, glm::vec4 color, float size)
        : position{position}, color{color}, size{size}
    {
    }
    Point(glm::vec2 position, glm::vec4 color, float size) : position{glm::vec3{position, 0.0f}}, color{color}, size{size}
    {
    }

    Point(glm::vec3 position) : position{position} {}
    Point(glm::vec2 position)
        : position{
              glm::vec3{position, 0.0f}
    }
    {
    }

    glm::vec3 position{};
    glm::vec4 color{0.0f, 1.0f, 0.0f, 1.0f}; // defalt color is green
    float size = 1.0f;

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};

class PointCollection
{
public:
    PointCollection(Device &device, size_t maxPointCount);

    PointCollection(const PointCollection &) = delete;
    PointCollection &operator=(const PointCollection &) = delete;

    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);

    void addPoint(const Point &point);
    void addPoints(const std::vector<Point> &points);
    void clearPoints();

    size_t pointCount() const { return points.size(); }

private:
    void createBuffers();
    void updateBuffer();

    Device &lveDevice;

    std::unique_ptr<Buffer> pointBuffer;
    std::unique_ptr<Buffer> stagingBuffer;
    std::vector<Point> points;
    size_t maxPointCount;
};
} // namespace lve