#pragma once

// libs
#include "include/glfw.hpp"

// std
#include <string>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <functional>

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

        KeyState getKeyState(int keyCode) const { return keyState.at(keyCode); }
        bool isKeyPressed(int keyCode) const { return keyState.at(keyCode) == KeyState::PRESS; }
        bool isKeyReleased(int keyCode) const { return keyState.at(keyCode) == KeyState::RELEASE; }
        bool isKeyRepeated(int keyCode) const { return keyState.at(keyCode) == KeyState::REPEAT; }
        bool isKeyUpdated(int keyCode) const { return keyUpdated.at(keyCode); }
        void clearKeyUpdate(int keyCode) { keyUpdated[keyCode] = false; }
        void oneTimeKeyUse(int keyCode, std::function<void()> callback, KeyState targetState=KeyState::PRESS);
        
        KeyState getMouseButtonState(int button) const { return mouseButtonState.at(button); }
        void getMousePosition(double &x, double &y) const { x = mouseX; y = mouseY; }
        void getMousePositionDelta(double &dx, double &dy) const { dx = mouseDeltaX; dy = mouseDeltaY; }
        bool isMouseButtonPressed(int button) const { return mouseButtonState.at(button) == KeyState::PRESS; }
        bool isMouseButtonReleased(int button) const { return mouseButtonState.at(button) == KeyState::RELEASE; }
        bool isMouseButtonUpdated(int button) const { return mouseButtonUpdated.at(button); }
        void clearMouseButtonUpdate(int button) { mouseButtonUpdated[button] = false; }
        bool isMouseMoved() const { return mouseDeltaX != 0 || mouseDeltaY != 0; }

    private:
        Input();

        double mouseX;
        double mouseY;
        double mouseDeltaX;
        double mouseDeltaY;

        std::unordered_map<int, KeyState> keyState;
        std::unordered_map<int, KeyState> mouseButtonState;

        std::unordered_map<int, bool> keyUpdated;
        std::unordered_map<int, bool> mouseButtonUpdated;

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
        void resize(int width, int height);

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
        void mainThreadGlfwEventLoop();

        Input input;
        
    private:
        void initWindow(int width, int height);

        std::string windowName;
        GLFWwindow *window;
    };
} // namespace lve
