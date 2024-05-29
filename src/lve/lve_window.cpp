#include "lve_window.hpp"

// std
#include <stdexcept>

namespace lve
{

    LveWindow::LveWindow(int w, int h, std::string name) : windowName{name}
    {
        initWindow(w, h);
    }

    LveWindow::~LveWindow()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void LveWindow::initWindow(int width, int height)
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to craete window surface");
        }
    }

    void LveWindow::mainThreadGlfwEventLoop()
    {
        bool wasMinimized = false;

        while (!shouldClose())
        {
            glfwPollEvents();

            bool isMinimized = isWindowMinimized();
            if (isMinimized)
            {
                glfwWaitEvents();
            }

            if (wasMinimized && !isMinimized)
            {
                std::lock_guard<std::mutex> lock(renderMutex);
                renderCondVar.notify_one(); // notify render thread to resume
            }

            wasMinimized = isMinimized;
        }
    }

    VkExtent2D LveWindow::getExtent()
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    bool LveWindow::isWindowMinimized()
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        return width == 0 || height == 0;
    }

    void LveWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto lveWindow = reinterpret_cast<LveWindow *>(glfwGetWindowUserPointer(window));
        // maybe useful in the future
    }

} // namespace lve
