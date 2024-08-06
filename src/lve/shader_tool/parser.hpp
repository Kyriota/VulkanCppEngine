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
        ShaderParser(const char *spvFileName);
        void dump(const char *outputFolder);

        struct SPIRTypeWrapper
        {
            SPIRTypeWrapper() = default;
            SPIRTypeWrapper(spirv_cross::SPIRType spirType);

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

            spirv_cross::SPIRType::BaseType baseType;
            spv::StorageClass storageClass;
            Image image;

            uint32_t bitWidth;
            uint32_t vecSize;
            uint32_t columns;

            template <class Archive> void serialize(Archive &archive)
            {
                archive(
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
            ShaderSummary(spirv_cross::CompilerGLSL &compiler, const std::string &shaderFileName);

            std::string shaderFileName;
            size_t rawShaderContentHash;
            std::vector<SPIRTypeWrapper> types;

            template <class Archive> void serialize(Archive &archive)
            {
                archive(CEREAL_NVP(shaderFileName), CEREAL_NVP(rawShaderContentHash));
            }

        private:
            void addTypesInResources(
                spirv_cross::SmallVector<spirv_cross::Resource> resources,
                spirv_cross::CompilerGLSL &compiler
            );
        };

    private:
        ShaderSummary summary;
        std::string shaderSrcName;
    };
} // namespace lve