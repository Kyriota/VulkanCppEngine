#include "lve/shader_tool/parser.hpp"

#include "lve/util/file_io.hpp"
#include "lve/util/hash.hpp"

// std
#include <cassert>
#include <fstream>

namespace lve
{
    ShaderParser::ShaderParser(const std::string &spvFilePath)
    {
        this->shaderSrcFilePath = spvFilePath;
        this->shaderSrcFilePath = this->shaderSrcFilePath.substr(
            0, this->shaderSrcFilePath.find_last_of(".")
        ); // get rid of .spv extension

        // load raw shader source and calculate hash
        std::vector<uint32_t> spirvBinary;
        io::readBinaryFile(spvFilePath, spirvBinary);
        spvBinaryHash = hash(spirvBinary);

        // check if meta file exists and if it has the same hash
        metaFilePath = shaderSrcFilePath + ".meta";
        if (io::fileExists(metaFilePath))
        {
            std::ifstream metaFile(metaFilePath, std::ios::binary);
            if (metaFile.is_open())
            {
                try // in case there is an error reading the meta file
                {
                    cereal::BinaryInputArchive archive(metaFile);
                    archive(summary); // load summary from meta file
                    if (summary.spvBinaryHash == spvBinaryHash)
                    {
                        return;
                    }
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error reading meta file: " << e.what() << std::endl;
                }
            }
        }

        // otherwise, parse the shader and generate meta file
        spirv_cross::CompilerGLSL compiler(std::move(spirvBinary));

        summary = ShaderSummary(compiler, spvFilePath, spvBinaryHash);
        dump(metaFilePath);
    }

    void ShaderParser::dump(const std::string &outputFilePath) const
    {
        std::ofstream outputFile(outputFilePath, std::ios::binary);

        if (!outputFile.is_open())
        {
            throw std::runtime_error("Failed to open output file: " + std::string(outputFilePath));
        }

        cereal::BinaryOutputArchive archive(outputFile);
        archive(summary);
    }

    ShaderParser::SPIRTypeWrapper::SPIRTypeWrapper(
        spirv_cross::SPIRType baseType,
        const std::string &name,
        bool isPushConstant
    )
        : name(name),
          isPushConstant(isPushConstant),
          baseType(baseType.basetype),
          storageClass(baseType.storage),
          bitWidth(baseType.width),
          vecSize(baseType.vecsize),
          columns(baseType.columns)
    {
        if (baseType.basetype == spirv_cross::SPIRType::Image)
        {
            image.dim = baseType.image.dim;
            image.depth = baseType.image.depth;
            image.arrayed = baseType.image.arrayed;
            image.ms = baseType.image.ms;
            image.sampled = baseType.image.sampled;
            image.format = baseType.image.format;
            image.access = baseType.image.access;
        }
    }

    ShaderParser::ShaderSummary::ShaderSummary(
        spirv_cross::CompilerGLSL &compiler,
        const std::string &shaderPath,
        const size_t spvBinaryHash
    )
        : shaderPath(shaderPath), spvBinaryHash(spvBinaryHash)
    {
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();
        addTypesInResources(resources.uniform_buffers, compiler);
        addTypesInResources(resources.storage_buffers, compiler);
        addTypesInResources(resources.stage_inputs, compiler);
        addTypesInResources(resources.stage_outputs, compiler);
        addTypesInResources(resources.subpass_inputs, compiler);
        addTypesInResources(resources.storage_images, compiler);
        addTypesInResources(resources.sampled_images, compiler);
        addTypesInResources(resources.atomic_counters, compiler);
        addTypesInResources(resources.separate_images, compiler);
        addTypesInResources(resources.separate_samplers, compiler);
        addTypesInResources(resources.push_constant_buffers, compiler, true);
    }

    void ShaderParser::ShaderSummary::addTypesInResources(
        spirv_cross::SmallVector<spirv_cross::Resource> resources,
        spirv_cross::CompilerGLSL &compiler,
        bool isPushConstant
    )
    {
        for (const spirv_cross::Resource &resource : resources)
        {
            SPIRTypeWrapper typeWrapper(
                compiler.get_type(resource.base_type_id), resource.name, isPushConstant
            );
            types.push_back(typeWrapper);
        }
    }
} // namespace lve