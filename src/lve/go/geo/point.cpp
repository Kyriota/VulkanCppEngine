#include "point.hpp"

namespace lve
{
std::vector<VkVertexInputBindingDescription> Point::getBindingDescriptions()
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Point);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Point::getAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

    attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Point, position)});
    attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Point, color)});
    attributeDescriptions.push_back({2, 0, VK_FORMAT_R32_SFLOAT, offsetof(Point, size)});

    return attributeDescriptions;
}

PointCollection::PointCollection(Device &device, size_t maxPointCount)
    : lveDevice{device}, maxPointCount{maxPointCount}
{
    points.reserve(maxPointCount);
    createBuffers();
}

void PointCollection::createBuffers()
{
    uint32_t pointSize = sizeof(Point);
    uint32_t totalPointSize = maxPointCount;

    stagingBuffer = std::make_unique<Buffer>(
        lveDevice,
        pointSize,
        totalPointSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBuffer->map();
    stagingBuffer->writeToBuffer((void *)points.data());

    pointBuffer = std::make_unique<Buffer>(
        lveDevice,
        pointSize,
        totalPointSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    pointBuffer->copyBufferFrom(stagingBuffer->getBuffer(), pointSize * maxPointCount);
}

void PointCollection::bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = {pointBuffer->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void PointCollection::draw(VkCommandBuffer commandBuffer)
{
    if (points.empty())
        return;

    vkCmdDraw(commandBuffer, static_cast<uint32_t>(points.size()), 1, 0, 0);
}

void PointCollection::addPoint(const Point &point)
{
    if (points.size() >= maxPointCount)
        throw std::runtime_error("Cannot add more points to collection");

    points.push_back(point);
    updateBuffer();
}

void PointCollection::addPoints(const std::vector<Point> &points)
{
    if (this->points.size() + points.size() > maxPointCount)
        throw std::runtime_error("Cannot add more points to collection");

    this->points.insert(this->points.end(), points.begin(), points.end());
    updateBuffer();
}

void PointCollection::clearPoints()
{
    points.clear();
    updateBuffer();
}

void PointCollection::updateBuffer()
{
    stagingBuffer->writeToBuffer((void *)points.data());
    pointBuffer->copyBufferFrom(stagingBuffer->getBuffer(), sizeof(Point) * maxPointCount);
}
} // namespace lve