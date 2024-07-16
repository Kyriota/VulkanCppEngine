#include "lve/core/window.hpp"

// std
#include <stdexcept>

namespace lve
{
    // Input Implementation
    Input::Input()
    {
        for (int i = 0; i < GLFW_KEY_LAST; i++)
        {
            keyState[i] = KeyState::RELEASE;
            keyUpdated[i] = false;
        }

        for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
        {
            mouseButtonState[i] = KeyState::RELEASE;
            mouseButtonUpdated[i] = false;
        }
    }

    void Input::oneTimeKeyUse(int keyCode, std::function<void()> callback, KeyState targetState)
    {
        if (keyUpdated[keyCode] && keyState[keyCode] == targetState)
        {
            callback();
            keyUpdated[keyCode] = false;
        }
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

        keyUpdated[key] = true;
    }

    void Input::handleMouseButtonEvent(int button, int action)
    {
        if (action == GLFW_PRESS)
            mouseButtonState[button] = KeyState::PRESS;
        else
            mouseButtonState[button] = KeyState::RELEASE;

        mouseButtonUpdated[button] = true;
    }

    void Input::handleCursorPositionEvent(double xpos, double ypos)
    {
        mouseDeltaX = xpos - mouseX;
        mouseDeltaY = ypos - mouseY;
        mouseX = xpos;
        mouseY = ypos;
    }

    // LveWindow Implementation
    Window::Window(int w, int h, std::string name) : windowName{name} { initWindow(w, h); }

    Window::~Window()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Window::initWindow(int width, int height)
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);

        // set callbacks
        glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
            Window *lveWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
            lveWindow->input.handleKeyEvent(key, action);
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods) {
            Window *lveWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
            lveWindow->input.handleMouseButtonEvent(button, action);
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos) {
            Window *lveWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
            lveWindow->input.handleCursorPositionEvent(xpos, ypos);
        });
    }

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to craete window surface");
        }
    }

    void Window::mainThreadGlfwEventLoop()
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

    VkExtent2D Window::getExtent()
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    bool Window::isWindowMinimized()
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        return width == 0 || height == 0;
    }

    void Window::setTitle(const std::string &title)
    {
        windowName = title;
        glfwSetWindowTitle(window, title.c_str());
    }

    void Window::resize(int width, int height) { glfwSetWindowSize(window, width, height); }
} // namespace lve
