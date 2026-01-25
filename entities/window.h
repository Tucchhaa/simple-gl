#pragma once

#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace SimpleGL {

class Input;

class Window : public std::enable_shared_from_this<Window> {
public:
    bool isCursorPosFixed = false;

    static std::shared_ptr<Window> create();

    ~Window();

    void open(int screenWidth, int screenHeight);
    void close() const;
    void destroy();

    std::shared_ptr<Input> input() const { return m_input; }

    // TODO: remove this in the future
    GLFWwindow* glfwWindow() const { return m_glfwWindow; }

    bool isOpen() const;
    bool isFocused() const;

    float aspectRatio() const { return static_cast<float>(m_frameWidth) / static_cast<float>(m_frameHeight); }
    int screenWidth() const { return m_screenWidth; }
    int screenHeight() const { return m_screenHeight; }
    int frameWidth() const { return m_frameWidth; }
    int frameHeight() const { return m_frameHeight; }

    static float time() { return static_cast<float>(glfwGetTime()); }

    void getCursorPos(double* mouseX, double* mouseY) const;
    void setCursorPosToCenter() const;

    void setTitle(const std::string& title);

    void pollEvents() const;

private:
    GLFWwindow* m_glfwWindow = nullptr;
    std::shared_ptr<Input> m_input;

    int m_screenWidth = 0;
    int m_screenHeight = 0;
    int m_frameWidth = 0;
    int m_frameHeight = 0;

    Window() = default;

    GLFWwindow* createGLFWWindow(int screenWidth, int screenHeight);

    void setEventCallbacks() const;
};

}
