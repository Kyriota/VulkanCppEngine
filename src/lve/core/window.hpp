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
    class Input
    {
    public:
        enum KeyState
        {
            RELEASE,
            PRESS,
            REPEAT
        };

        Input(const Input &) = delete;
        Input &operator=(const Input &) = delete;

        bool isKeyPressed(int keyCode) const { return keyState.at(keyCode); }
        bool isMouseButtonPressed(int button) const { return mouseButtonState.at(button); }
        void getMousePosition(double &x, double &y) const { x = mouseX; y = mouseY; }
        void getMousePositionDelta(double &dx, double &dy) const { dx = mouseDeltaX; dy = mouseDeltaY; }
        bool isMouseMoved() const { return mouseDeltaX != 0 || mouseDeltaY != 0; }

    private:
        Input();

        double mouseX;
        double mouseY;
        double mouseDeltaX;
        double mouseDeltaY;

        std::unordered_map<int, KeyState> keyState;
        std::unordered_map<int, KeyState> mouseButtonState;

        void handleKeyEvent(int key, int action);
        void handleMouseButtonEvent(int button, int action);
        void handleCursorPositionEvent(double xpos, double ypos);

        friend class Window;
    };

    class Window
    {
    public:
        Window(int w, int h, std::string name);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        std::mutex renderMutex;
        std::condition_variable renderCondVar;

        bool shouldClose() { return glfwWindowShouldClose(window); }
        VkExtent2D getExtent();
        bool isWindowMinimized();
        GLFWwindow *getGLFWwindow() const { return window; }
        void setTitle(const std::string &title);

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
        void mainThreadGlfwEventLoop();

        Input input;
        
    private:
        void initWindow(int width, int height);

        std::string windowName;
        GLFWwindow *window;
    };
} // namespace lve
