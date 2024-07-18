#include "lve/shader_tool/spirv_parser.hpp"

// std
#include <iostream>

namespace lve
{
    SpirvParser::SpirvParser(const std::vector<uint32_t> &spirv_binary)
    {
        spirv_cross::CompilerGLSL compiler(std::move(spirv_binary));
        resources = compiler.get_shader_resources();

        // Print input resources
        for (auto &resource : resources.stage_inputs)
        {
            std::cout << "Input: " << resource.name << std::endl;
            spirv_cross::SPIRType type = compiler.get_type(resource.type_id);
            continue;
        }

        // Print output resources
        for (auto &resource : resources.stage_outputs)
        {
            std::cout << "Output: " << resource.name << std::endl;
            spirv_cross::SPIRType type = compiler.get_type(resource.type_id);
            continue;
        }

        // Print uniform resources
        for (auto &resource : resources.uniform_buffers)
        {
            std::cout << "Uniform: " << resource.name << std::endl;
            spirv_cross::SPIRType type = compiler.get_type(resource.base_type_id);
            continue;
        }

        // Print texture resources
        for (auto &resource : resources.sampled_images)
        {
            std::cout << "Texture: " << resource.name << std::endl;
            spirv_cross::SPIRType type = compiler.get_type(resource.base_type_id);
            continue;
        }

        // Print sampler resources
        for (auto &resource : resources.separate_samplers)
        {
            std::cout << "Sampler: " << resource.name << std::endl;
            spirv_cross::SPIRType type = compiler.get_type(resource.base_type_id);
            continue;
        }

        // Print image resources
        for (auto &resource : resources.storage_images)
        {
            std::cout << "Image: " << resource.name << std::endl;
            spirv_cross::SPIRType type = compiler.get_type(resource.base_type_id);
            continue;
        }

        // Print push constant resources
        for (auto &resource : resources.push_constant_buffers)
        {
            std::cout << "Push Constant: " << resource.name << std::endl;
            spirv_cross::SPIRType type = compiler.get_type(resource.base_type_id);
            continue;
        }

        // Print buffer resources
        for (auto &resource : resources.storage_buffers)
        {
            std::cout << "Buffer: " << resource.name << std::endl;
            spirv_cross::SPIRType type = compiler.get_type(resource.base_type_id);
            continue;
        }
    }
} // namespace lve