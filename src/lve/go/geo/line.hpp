#pragma once

#include "lve/core/device.hpp"
#include "lve/core/resource/buffer.hpp"

// libs
#include "include/glm.hpp"

// std
#include <memory>
#include <vector>

namespace lve
{
struct Line
{
    struct Vertex
    {
        Vertex() = default;

        Vertex(glm::vec3 position, glm::vec4 color) : position{position}, color{color} {}
        Vertex(glm::vec2 position, glm::vec4 color) : position{glm::vec3{position, 0.0f}}, color{color}
        {
        }

        Vertex(glm::vec3 position) : position{position} {}
        Vertex(glm::vec2 position)
            : position{
                  glm::vec3{position, 0.0f}
        }
        {
        }

        glm::vec3 position{};
        glm::vec4 color{0.0f, 1.0f, 0.0f, 1.0f}; // defalt color is green

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    Line() = default;

    Line(Vertex start, Vertex end) : start{start}, end{end} {}

    Vertex start;
    Vertex end;
};

class LineCollection
{
public:
    LineCollection(Device &device, size_t maxLineCount);

    LineCollection(const LineCollection &) = delete;
    LineCollection &operator=(const LineCollection &) = delete;

    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);

    void addLine(const Line &line);
    void addLines(const std::vector<Line> &lines);
    void clearLines();

    size_t getLineCount() const { return lineCount; }

private:
    void createLineBuffer();
    void updateBuffer();

    Device &lveDevice;

    std::unique_ptr<Buffer> lineBuffer;
    std::unique_ptr<Buffer> stagingBuffer;
    std::vector<Line> lines;
    size_t lineCount{0};
    size_t maxLineCount;
};
} // namespace lve