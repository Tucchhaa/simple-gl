#include "window.h"
#include "input.h"

namespace SimpleGL {

std::shared_ptr<Input> Input::create(const std::weak_ptr<Window>& window) {
    return std::shared_ptr<Input>(new Input(window));
}

void Input::keyCallback(int key, int action) {
    if (action == GLFW_PRESS) {
        setKeyState(key, true);
    } else if (action == GLFW_RELEASE) {
        setKeyState(key, false);
    }
}

void Input::mouseButtonCallback(int button, int action) {
    m_currentMouseStates[button] = action == GLFW_PRESS;
}

void Input::process() {
    m_previousKeyStates = m_currentKeyStates;
    m_previousMouseStates = m_currentMouseStates;

    updateCursorPosition();
    updateDeltaTime();

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
    float axisHorizontal = static_cast<int>(isKeyDown(GLFW_KEY_D)) - static_cast<int>(isKeyDown(GLFW_KEY_A));
    float axisVertical = static_cast<int>(isKeyDown(GLFW_KEY_W)) - static_cast<int>(isKeyDown(GLFW_KEY_S));

    return { axisHorizontal, axisVertical };
}

glm::vec2 Input::mouseDelta() const {
    const float screenWidth = static_cast<float>(window()->screenWidth());
    const float screenHeight = static_cast<float>(window()->screenHeight());

    auto xMouseDelta = (m_mouseX - screenWidth / 2.0f) / screenWidth;
    auto yMouseDelta = (m_mouseY - screenHeight / 2.0f) / screenHeight;

    return { xMouseDelta, yMouseDelta };
}

void Input::updateCursorPosition() {
    glfwGetCursorPos(window()->glfwWindow(), &m_mouseX, &m_mouseY);
}

void Input::setKeyState(int key, bool pressed) {
    m_currentKeyStates[key] = pressed;
}

void Input::updateDeltaTime() {
    m_deltaTime = Window::time() - m_lastFrameTime;
    m_lastFrameTime = Window::time();
}

}
