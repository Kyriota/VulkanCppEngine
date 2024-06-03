#include "lve_window.hpp"

// std
#include <stdexcept>

namespace lve
{
    // Input Implementation
    Input::Input()
    {
        for (int i = 0; i < GLFW_KEY_LAST; i++)
            keyState[i] = KeyState::RELEASE;
        
        for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
            mouseButtonState[i] = KeyState::RELEASE;
    }

    void Input::handleKeyEvent(int key, int action)
    {
        if (key == GLFW_KEY_UNKNOWN)
            return;
        
        if (action == GLFW_REPEAT)
            keyState[key] = KeyState::REPEAT;
        else if (action == GLFW_RELEASE)
            keyState[key] = KeyState::RELEASE;
        else
            keyState[key] = KeyState::PRESS;
    }

    void Input::handleMouseButtonEvent(int button, int action)
    {
        if (action == GLFW_PRESS)
            mouseButtonState[button] = KeyState::PRESS;
        else
            mouseButtonState[button] = KeyState::RELEASE;
    }

    void Input::handleCursorPositionEvent(double xpos, double ypos)
    {
        mouseDeltaX = xpos - mouseX;
        mouseDeltaY = ypos - mouseY;
        mouseX = xpos;
        mouseY = ypos;
    }

    // LveWindow Implementation
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

        // set callbacks
        glfwSetKeyCallback(
            window,
            [](GLFWwindow *window, int key, int scancode, int action, int mods)
            {
                LveWindow *lveWindow = reinterpret_cast<LveWindow *>(glfwGetWindowUserPointer(window));
                lveWindow->input.handleKeyEvent(key, action);
            });

        glfwSetMouseButtonCallback(
            window,
            [](GLFWwindow *window, int button, int action, int mods)
            {
                LveWindow *lveWindow = reinterpret_cast<LveWindow *>(glfwGetWindowUserPointer(window));
                lveWindow->input.handleMouseButtonEvent(button, action);
            });

        glfwSetCursorPosCallback(
            window,
            [](GLFWwindow *window, double xpos, double ypos)
            {
                LveWindow *lveWindow = reinterpret_cast<LveWindow *>(glfwGetWindowUserPointer(window));
                lveWindow->input.handleCursorPositionEvent(xpos, ypos);
            });
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

    void LveWindow::setTitle(const std::string &title)
    {
        windowName = title;
        glfwSetWindowTitle(window, title.c_str());
    }
} // namespace lve
