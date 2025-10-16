#pragma once

#include <memory>
#include <string>
#include <utility>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace SimpleGL {

class Input;

class Window {
public:
    const std::string label;

    bool isCursorPositionFixed;

    static std::shared_ptr<Window> create(const std::string &label, GLFWwindow* glfwWindow);

    ~Window();

    GLFWwindow* glfwWindow() const { return m_glfwWindow; }

    void makeCurrent() const;

    void beforeFrameRendered() const;

    void afterFrameRendered() const;

    std::shared_ptr<Input>& input() { return m_input; }

    void close() const;

    void setTitle(const std::string& title) const;

    bool isOpen() const;

    bool isFocused() const;

    float aspectRatio() const { return static_cast<float>(m_frameWidth) / static_cast<float>(m_frameHeight); }

    int screenWidth() const { return m_screenWidth; }
    int screenHeight() const { return m_screenHeight; }
    int frameWidth() const { return m_frameWidth; }
    int frameHeight() const { return m_frameHeight; }

    static float time() { return static_cast<float>(glfwGetTime()); }

    void setCursorPositionToCenter() const;

private:
    GLFWwindow* m_glfwWindow = nullptr;

    std::shared_ptr<Input> m_input;

    int m_screenWidth = 0;
    int m_screenHeight = 0;
    int m_frameWidth = 0;
    int m_frameHeight = 0;

    Window(std::string label, GLFWwindow* glfwWindow): label(std::move(label)), m_glfwWindow(glfwWindow) {}

    void setCallbacks() const;
};

}

