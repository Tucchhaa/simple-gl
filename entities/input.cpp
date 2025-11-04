#include "window.h"
#include "input.h"

namespace SimpleGL {

std::shared_ptr<Input> Input::create(const std::weak_ptr<Window>& window) {
    auto instance = std::shared_ptr<Input>(new Input(window));

    return instance;
}

void Input::process() {
    m_previousKeyStates = m_currentKeyStates;
    m_previousMouseStates = m_currentMouseStates;

    for (int key = 0; key <= GLFW_KEY_LAST; ++key) {
        m_currentKeyStates[key] = (glfwGetKey(window()->glfwWindow(), key) == GLFW_PRESS);
    }
    for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; ++button) {
        m_currentMouseStates[button] = (glfwGetMouseButton(window()->glfwWindow(), button) == GLFW_PRESS);
    }

    updateCursorPosition();
    updateDeltaTime();

    if(!window()->isFocused()) {
        reset();
        return;
    }

    m_yPositivePressed = isKeyDown(GLFW_KEY_W);
    m_yNegativePressed = isKeyDown(GLFW_KEY_S);
    m_xPositivePressed = isKeyDown(GLFW_KEY_D);
    m_xNegativePressed = isKeyDown(GLFW_KEY_A);

    if (window()->isCursorPositionFixed) {
        window()->setCursorPositionToCenter();
    }
}

bool Input::isKeyDown(const int key) const {
    return m_currentKeyStates[key];
}

bool Input::isKeyPressed(const int key) const {
    return m_currentKeyStates[key] && !m_previousKeyStates[key];
}

bool Input::isKeyReleased(const int key) const {
    return !m_currentKeyStates[key] && m_previousKeyStates[key];
}

bool Input::isMouseButtonDown(const int button) const {
    return m_currentMouseStates[button];
}

bool Input::isMouseButtonPressed(const int button) const {
    return m_currentMouseStates[button] && !m_previousMouseStates[button];
}

bool Input::isMouseButtonReleased(const int button) const {
    return !m_currentMouseStates[button] && m_previousMouseStates[button];
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
