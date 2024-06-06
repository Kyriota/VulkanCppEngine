#include "lve/go/geo/line.hpp"

// std
#include <memory>
#include <vector>
#include <stdexcept>

namespace lve
{
    std::vector<VkVertexInputBindingDescription> Line::Vertex::getBindingDescriptions()
    {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> Line::Vertex::getAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color)});

        return attributeDescriptions;
    }

    LineCollection::LineCollection(Device &device, size_t maxLineCount)
        : lveDevice{device}, maxLineCount{maxLineCount}
    {
        lines.reserve(maxLineCount);
        createLineBuffer();
    }

    void LineCollection::createLineBuffer()
    {
        uint32_t lineSize = sizeof(Line);
        uint32_t totalLineCount = maxLineCount;

        stagingBuffer = std::make_unique<Buffer>(
            lveDevice,
            lineSize,
            totalLineCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        stagingBuffer->map();
        stagingBuffer->writeToBuffer((void *)lines.data());

        lineBuffer = std::make_unique<Buffer>(
            lveDevice,
            lineSize,
            totalLineCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        lineBuffer->copyBufferFrom(stagingBuffer->getBuffer(), lineSize * maxLineCount);
    }

    void LineCollection::bind(VkCommandBuffer commandBuffer)
    {
        VkBuffer buffers[] = {lineBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    }

    void LineCollection::draw(VkCommandBuffer commandBuffer)
    {
        if (lineCount == 0)
            return;

        vkCmdDraw(commandBuffer, static_cast<uint32_t>(lineCount * 2), 1, 0, 0);
    }

    void LineCollection::addLine(const Line &line)
    {
        if (lineCount >= maxLineCount)
            throw std::runtime_error("Cannot add more lines to LineCollection than maxLineCount");

        lines.push_back(line);
        lineCount++;
        updateBuffer();
    }

    void LineCollection::addLines(const std::vector<Line> &lines)
    {
        if (lineCount + lines.size() > maxLineCount)
            throw std::runtime_error("Cannot add more lines to LineCollection than maxLineCount");

        this->lines.insert(this->lines.end(), lines.begin(), lines.end());
        lineCount += lines.size();
        updateBuffer();
    }

    void LineCollection::clearLines()
    {
        lines.clear();
        lineCount = 0;
        updateBuffer();
    }

    void LineCollection::updateBuffer()
    {
        if (lineCount == 0)
            return;

        stagingBuffer->writeToBuffer((void *)lines.data());
        lineBuffer->copyBufferFrom(stagingBuffer->getBuffer(), sizeof(Line) * maxLineCount);
    }
} // namespace lve