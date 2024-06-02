#pragma once

// libs
#include "include/glfw.hpp"

// std
#include <string>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

namespace lve
{
    class LveInput
    {
    public:
        enum KeyState
        {
            RELEASE,
            PRESS,
            REPEAT
        };

        LveInput(const LveInput &) = delete;
        LveInput &operator=(const LveInput &) = delete;

        bool isKeyPressed(int keyCode) const { return keyState.at(keyCode); }
        bool isMouseButtonPressed(int button) const { return mouseButtonState.at(button); }
        void getMousePosition(double &x, double &y) const { x = mouseX; y = mouseY; }
        void getMousePositionDelta(double &dx, double &dy) const { dx = mouseDeltaX; dy = mouseDeltaY; }

    private:
        LveInput();

        double mouseX;
        double mouseY;
        double mouseDeltaX;
        double mouseDeltaY;

        std::unordered_map<int, KeyState> keyState;
        std::unordered_map<int, KeyState> mouseButtonState;

        void handleKeyEvent(int key, int action);
        void handleMouseButtonEvent(int button, int action);
        void handleCursorPositionEvent(double xpos, double ypos);

        friend class LveWindow;
    };

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
        void initWindow(int width, int height);

        std::string windowName;
        GLFWwindow *window;

        LveInput lveInput;
    };
} // namespace lve
