#include "input.h"

#include "window.h"

namespace SimpleGL {

Input::Input(Window* window, GLFWwindow* glfwWindow):
    m_window(window),
    m_glfwWindow(glfwWindow)
{ }

void Input::reset() {
    m_lastFrameTime = Window::time();
    m_deltaTime = 0;

    m_yPositivePressed = 0;
    m_yNegativePressed = 0;
    m_xPositivePressed = 0;
    m_xNegativePressed = 0;
}

void Input::process() {
    updateCursorPosition();

    if(m_window->isFocused()) {
        reset();
        return;
    }

    m_yPositivePressed = isKeyPressed(GLFW_KEY_W);
    m_yNegativePressed = isKeyPressed(GLFW_KEY_S);
    m_xPositivePressed = isKeyPressed(GLFW_KEY_D);
    m_xNegativePressed = isKeyPressed(GLFW_KEY_A);

    if (m_window->isCursorPositionFixed()) {
        setCursorPositionToCenter();
    }
}

bool Input::isKeyPressed(const int key) const {
    return glfwGetKey(m_glfwWindow, key) == GLFW_PRESS;
}

bool Input::isMouseButtonPressed(const int button) const {
    return glfwGetMouseButton(m_glfwWindow, button) == GLFW_PRESS;
}

glm::vec2 Input::axisVec2() const {
    float axisHorizontal = m_xPositivePressed - m_xNegativePressed;
    float axisVertical = m_yPositivePressed - m_yNegativePressed;

    return { axisHorizontal, axisVertical };
}

glm::vec2 Input::mouseDelta() const {
    auto xMouseDelta = static_cast<float>(m_mouseX - m_window->frameWidth() / 2.0);
    auto yMouseDelta = static_cast<float>(m_mouseY - m_window->frameHeight() / 2.0);

    return { xMouseDelta, yMouseDelta };
}

void Input::updateCursorPosition() {
    glfwGetCursorPos(m_glfwWindow, &m_mouseX, &m_mouseY);
}

void Input::updateDeltaTime() {
    m_deltaTime = Window::time() - m_lastFrameTime;
    m_lastFrameTime = Window::time();
}

void Input::setCursorPositionToCenter() const {
    const auto xPosition = m_window->frameWidth() / 2;
    const auto yPosition = m_window->frameHeight() / 2;

    glfwSetCursorPos(m_glfwWindow, xPosition, yPosition);
}

}
