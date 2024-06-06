#pragma once

#include "lve/core/resource/buffer.hpp"
#include "lve/core/device.hpp"

// libs
#include "include/glm.hpp"

// std
#include <memory>
#include <vector>

namespace lve
{
    class Model
    {
    public:
        struct Vertex
        {
            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex &other) const
            {
                return position == other.position && color == other.color && normal == other.normal &&
                       uv == other.uv;
            }
        };

        struct Builder
        {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string &filepath);
        };

        Model(Device &device, const Model::Builder &builder);

        Model(const Model &) = delete;
        Model &operator=(const Model &) = delete;

        static std::unique_ptr<Model> createModelFromFile(
            Device &device, const std::string &filepath);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffer(const std::vector<Vertex> &vertices);
        void createIndexBuffer(const std::vector<uint32_t> &indices);

        Device &lveDevice;

        std::unique_ptr<Buffer> vertexBuffer;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        std::unique_ptr<Buffer> indexBuffer;
        uint32_t indexCount;
    };
} // namespace lve
