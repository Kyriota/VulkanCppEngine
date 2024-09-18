#pragma once

// libs
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <spirv_cross/spirv_glsl.hpp>

// std
#include <string>
#include <vector>

namespace lve
{
    class ShaderParser
    {
    public:
        struct SPIRTypeWrapper
        {
            SPIRTypeWrapper() = default;
            SPIRTypeWrapper(
                spirv_cross::SPIRType spirType,
                spv::StorageClass storageClass,
                const std::string &name,
                int binding,
                int location,
                uint32_t set
            );

            struct Image
            {
                uint32_t dim;
                bool depth;
                bool arrayed;
                bool ms; // multisampled
                uint32_t sampled;
                spv::ImageFormat format;
                spv::AccessQualifier access;

                template <class Archive> void serialize(Archive &archive)
                {
                    archive(
                        CEREAL_NVP(dim),
                        CEREAL_NVP(depth),
                        CEREAL_NVP(arrayed),
                        CEREAL_NVP(ms),
                        CEREAL_NVP(sampled),
                        CEREAL_NVP(format),
                        CEREAL_NVP(access)
                    );
                }
            };

            std::string name;
            int binding;
            int location;
            uint32_t set;
            spirv_cross::SPIRType::BaseType baseType;
            spv::StorageClass storageClass; // uniform, input, output, push_constant, etc.
            Image image;

            uint32_t bitWidth;
            uint32_t vecSize;
            uint32_t columns;

            template <class Archive> void serialize(Archive &archive)
            {
                archive(
                    CEREAL_NVP(name),
                    CEREAL_NVP(binding),
                    CEREAL_NVP(baseType),
                    CEREAL_NVP(storageClass),
                    CEREAL_NVP(image),
                    CEREAL_NVP(bitWidth),
                    CEREAL_NVP(vecSize),
                    CEREAL_NVP(columns)
                );
            }
        };

        struct ShaderSummary
        {
        public:
            ShaderSummary() = default;
            ShaderSummary(
                spirv_cross::CompilerGLSL &compiler,
                const std::string &spvFilePath,
                const size_t spvBinaryHash
            );

            std::string spvFilePath;
            size_t spvBinaryHash;
            std::vector<SPIRTypeWrapper> variables;

            template <class Archive> void serialize(Archive &archive)
            {
                archive(CEREAL_NVP(spvFilePath), CEREAL_NVP(spvBinaryHash), CEREAL_NVP(variables));
            }

        private:
            void addTypesInResources(
                spirv_cross::SmallVector<spirv_cross::Resource> resources,
                spirv_cross::CompilerGLSL &compiler
            );
        };

        ShaderParser(const std::string &spvFilePath);
        ShaderSummary getSummary() const { return summary; }

    private:
        void dump(const std::string &outputFilePath) const;

        ShaderSummary summary;
        std::string shaderSrcFilePath;
        std::string metaFilePath;
        size_t spvBinaryHash;
    };
} // namespace lve