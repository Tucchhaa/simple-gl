#include "input.h"

#include <utility>

#include "window.h"

namespace SimpleGL {

std::shared_ptr<Input> Input::create(const std::weak_ptr<Window>& window) {
    auto instance = std::shared_ptr<Input>(new Input(window));

    return instance;
}

void Input::process() {
    updateCursorPosition();

    if(window()->isFocused()) {
        reset();
        return;
    }

    m_yPositivePressed = isKeyPressed(GLFW_KEY_W);
    m_yNegativePressed = isKeyPressed(GLFW_KEY_S);
    m_xPositivePressed = isKeyPressed(GLFW_KEY_D);
    m_xNegativePressed = isKeyPressed(GLFW_KEY_A);

    if (window()->isCursorPositionFixed()) {
        setCursorPositionToCenter();
    }
}

bool Input::isKeyPressed(const int key) const {
    return glfwGetKey(window()->glfwWindow(), key) == GLFW_PRESS;
}

bool Input::isMouseButtonPressed(const int button) const {
    return glfwGetMouseButton(window()->glfwWindow(), button) == GLFW_PRESS;
}

glm::vec2 Input::axisVec2() const {
    float axisHorizontal = m_xPositivePressed - m_xNegativePressed;
    float axisVertical = m_yPositivePressed - m_yNegativePressed;

    return { axisHorizontal, axisVertical };
}

glm::vec2 Input::mouseDelta() const {
    auto xMouseDelta = static_cast<float>(m_mouseX - window()->frameWidth() / 2.0);
    auto yMouseDelta = static_cast<float>(m_mouseY - window()->frameHeight() / 2.0);

    return { xMouseDelta, yMouseDelta };
}

void Input::reset() {
    m_lastFrameTime = Window::time();
    m_deltaTime = 0;

    m_yPositivePressed = 0;
    m_yNegativePressed = 0;
    m_xPositivePressed = 0;
    m_xNegativePressed = 0;
}

void Input::updateCursorPosition() {
    glfwGetCursorPos(window()->glfwWindow(), &m_mouseX, &m_mouseY);
}

void Input::updateDeltaTime() {
    m_deltaTime = Window::time() - m_lastFrameTime;
    m_lastFrameTime = Window::time();
}

void Input::setCursorPositionToCenter() const {
    const auto xPosition = window()->frameWidth() / 2;
    const auto yPosition = window()->frameHeight() / 2;

    glfwSetCursorPos(window()->glfwWindow(), xPosition, yPosition);
}

}
