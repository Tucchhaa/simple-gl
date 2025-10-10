#include "input.h"

#include "window.h"

namespace SimpleGL {

std::shared_ptr<Input> Input::create(const std::weak_ptr<Window>& window) {
    auto instance = std::shared_ptr<Input>(new Input(window));

    return instance;
}

void Input::process() {
    updateCursorPosition();
    updateDeltaTime();

    if(!window()->isFocused()) {
        reset();
        return;
    }

    m_yPositivePressed = isKeyPressed(GLFW_KEY_W);
    m_yNegativePressed = isKeyPressed(GLFW_KEY_S);
    m_xPositivePressed = isKeyPressed(GLFW_KEY_D);
    m_xNegativePressed = isKeyPressed(GLFW_KEY_A);

    if (window()->isCursorPositionFixed) {
        window()->setCursorPositionToCenter();
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
    const float screenWidth = static_cast<float>(window()->screenWidth());
    const float screenHeight = static_cast<float>(window()->screenHeight());

    auto xMouseDelta = (m_mouseX - screenWidth / 2.0f) / screenWidth;
    auto yMouseDelta = (m_mouseY - screenHeight / 2.0f) / screenHeight;

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

}
