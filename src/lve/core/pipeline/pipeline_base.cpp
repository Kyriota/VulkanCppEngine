#include "lve/core/pipeline/pipeline_base.hpp"

#include "lve/util/file_io.hpp"

namespace lve
{
    Pipeline::Pipeline(Pipeline &&other) : lveDevice(other.lveDevice)
    {
        pipeline = other.pipeline;
        pipelineLayout = other.pipelineLayout;
        for (auto &shaderModule : other.shaderModules)
        {
            shaderModules[shaderModule.first] = shaderModule.second;
        }
        initialized = other.initialized;

        // Reset other object
        other.pipeline = VK_NULL_HANDLE;
        other.pipelineLayout = VK_NULL_HANDLE;
        for (auto &shaderModule : other.shaderModules)
        {
            shaderModule.second = VK_NULL_HANDLE;
        }
        other.initialized = false;
    }

    Pipeline &Pipeline::operator=(Pipeline &&other)
    {
        if (this->lveDevice.vkDevice() != other.lveDevice.vkDevice())
        {
            throw std::runtime_error("Moved Pipeline objects must be on the same VkDevice");
        }

        if (this != &other)
        {
            cleanUp();

            pipeline = other.pipeline;
            pipelineLayout = other.pipelineLayout;
            for (auto &shaderModule : other.shaderModules)
            {
                shaderModules[shaderModule.first] = shaderModule.second;
            }
            initialized = other.initialized;

            // Reset other object
            other.pipeline = VK_NULL_HANDLE;
            other.pipelineLayout = VK_NULL_HANDLE;
            for (auto &shaderModule : other.shaderModules)
            {
                shaderModule.second = VK_NULL_HANDLE;
            }
            other.initialized = false;
        }

        return *this;
    }

    void Pipeline::cleanUp()
    {
        if (initialized)
        {
            vkDestroyPipeline(lveDevice.vkDevice(), pipeline, nullptr);
            vkDestroyPipelineLayout(lveDevice.vkDevice(), pipelineLayout, nullptr);
            for (auto &shaderModule : shaderModules)
            {
                vkDestroyShaderModule(lveDevice.vkDevice(), shaderModule.second, nullptr);
            }
            initialized = false;
        }
    }

    void Pipeline::initShaderModule(std::string moduleName, const std::vector<char> &code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(lveDevice.vkDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }

        shaderModules[moduleName] = shaderModule;
    }
} // namespace lve