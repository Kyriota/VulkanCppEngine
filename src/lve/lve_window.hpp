#pragma once

// libs
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <string>
#include <mutex>
#include <condition_variable>

namespace lve
{

    class LveWindow
    {
    public:
        LveWindow(int w, int h, std::string name);
        ~LveWindow();

        LveWindow(const LveWindow &) = delete;
        LveWindow &operator=(const LveWindow &) = delete;

        std::mutex renderMutex;
        std::condition_variable renderCondVar;

        bool shouldClose() { return glfwWindowShouldClose(window); }
        VkExtent2D getExtent();
        bool isWindowMinimized();
        GLFWwindow *getGLFWwindow() const { return window; }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
        void mainThreadGlfwEventLoop();

    private:
        static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
        void initWindow(int width, int height);

        std::string windowName;
        GLFWwindow *window;
    };
} // namespace lve
