#pragma once

#include <memory>
#include <string>
#include <utility>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "input.h"

namespace SimpleGL {

class Window {
public:
    const std::string label;

    static std::shared_ptr<Window> create(const std::string &label, GLFWwindow* glfwWindow);

    ~Window();

    GLFWwindow* glfwWindow() const { return m_glfwWindow; }

    void makeCurrent() const;

    void beforeFrameRendered() const;

    void afterFrameRendered() const;

    std::shared_ptr<Input> input() { return m_input; }

    void close() const;

    void setTitle(const std::string& title) const;

    bool isOpen() const;

    bool isFocused() const;

    bool isCursorPositionFixed() const { return m_isCursorPositionFixed; }

    float aspectRatio() const { return static_cast<float>(m_frameWidth) / static_cast<float>(m_frameHeight); }

    int frameWidth() const { return m_frameWidth; }

    int frameHeight() const { return m_frameHeight; }

    static float time() { return static_cast<float>(glfwGetTime()); }

private:
    GLFWwindow* m_glfwWindow = nullptr;

    std::shared_ptr<Input> m_input;

    int m_frameWidth = 0;
    int m_frameHeight = 0;
    bool m_isCursorPositionFixed = false;

    Window(std::string label, GLFWwindow* glfwWindow): label(std::move(label)), m_glfwWindow(glfwWindow) {}

    void setCallbacks() const;
};

}

