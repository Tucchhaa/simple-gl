#pragma once

#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "input.h"

namespace SimpleGL {

class Window {
    friend class WindowManager;

public:
    const std::string label;

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
    GLFWwindow* m_glfwWindow;
    std::shared_ptr<Input> m_input;

    int m_frameWidth;
    int m_frameHeight;
    bool m_isCursorPositionFixed = false;

    explicit Window(std::string  label, GLFWwindow* glfwWindow);

    void setCallbacks() const;
};

}

