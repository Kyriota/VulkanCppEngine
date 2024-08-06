#include "lve/shader_tool/parser.hpp"

#include "lve/util/file_io.hpp"

// std
#include <cassert>
#include <fstream>

namespace lve
{
    ShaderParser::ShaderParser(const char *spvFileName)
    {
        std::vector<uint32_t> spirvBinary;
        lve::io::readBinaryFile("assets/shaders/simple_shader.frag.spv", spirvBinary);
        spirv_cross::CompilerGLSL compiler(std::move(spirvBinary));

        shaderSrcName = spvFileName;
        shaderSrcName =
            shaderSrcName.substr(0, shaderSrcName.find_last_of(".")); // get rid of .spv extension

        summary = ShaderSummary(compiler, spvFileName);
    }

    void ShaderParser::dump(const char *outputFolder)
    {
        // check if the output folder exists
        assert(io::pathExists(outputFolder));

        // check if the output folder ends with a slash
        std::string outputFolderStr(outputFolder);
        if (outputFolderStr.back() != '/')
        {
            outputFolderStr += '/';
        }

        std::string outputFilePath = outputFolderStr + shaderSrcName + ".sum";
        std::ofstream outputFile(outputFilePath, std::ios::binary);
        cereal::BinaryOutputArchive archive(outputFile);
        archive(summary);
    }

    ShaderParser::SPIRTypeWrapper::SPIRTypeWrapper(spirv_cross::SPIRType baseType)
        : baseType(baseType.basetype),
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
        const std::string &shaderFileName
    )
        : shaderFileName(shaderFileName)
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
        addTypesInResources(resources.push_constant_buffers, compiler);
        addTypesInResources(resources.separate_images, compiler);
        addTypesInResources(resources.separate_samplers, compiler);

        // load raw shader source and calculate hash
        std::vector<char> shaderSource;
        io::readBinaryFile(shaderFileName, shaderSource);
        rawShaderContentHash =
            std::hash<std::string>{}(std::string(shaderSource.begin(), shaderSource.end()));
    }

    void ShaderParser::ShaderSummary::addTypesInResources(
        spirv_cross::SmallVector<spirv_cross::Resource> resources,
        spirv_cross::CompilerGLSL &compiler
    )
    {
        for (const spirv_cross::Resource &resource : resources)
        {
            SPIRTypeWrapper typeWrapper(compiler.get_type(resource.base_type_id));
            types.push_back(typeWrapper);
        }
    }
} // namespace lve