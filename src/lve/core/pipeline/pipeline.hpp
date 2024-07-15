#pragma once

#include "lve/core/device.hpp"

namespace lve
{
    class Pipeline
    {
    public:
        Pipeline(Device &device) : lveDevice(device) {}

        VkPipeline getPipeline() { return pipeline; }
        VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

    protected:
        void cleanUp()
        {
            if (initialized)
            {
                release();
                initialized = false;
            }
        }
        virtual void release() = 0;

        Device &lveDevice;
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;

        bool initialized = false;
    };
} // namespace lve