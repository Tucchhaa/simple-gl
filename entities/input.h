#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

namespace SimpleGL {

class Window;

class Input {
    friend class Window;

public:
    glm::vec2 axisVec2() const;

    glm::vec2 mouseDelta() const;

    bool isKeyPressed(int key) const;

    bool isMouseButtonPressed(int button) const;

    float getDeltaTime() const { return m_deltaTime; }

private:
    /// No need to delete these pointers. They are disposed in corresponding Window
    Window* m_window;
    GLFWwindow* m_glfwWindow;

    float m_lastFrameTime = 0;
    float m_deltaTime = 0;

    float m_yPositivePressed = 0;
    float m_yNegativePressed = 0;
    float m_xPositivePressed = 0;
    float m_xNegativePressed = 0;

    double m_mouseX = 0;
    double m_mouseY = 0;

    Input(Window* window, GLFWwindow* glfwWindow);

    void process();

    void reset();

    void updateCursorPosition();

    void updateDeltaTime();

    void setCursorPositionToCenter() const;
};

}
