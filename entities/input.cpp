#include "window.h"
#include "input.h"

namespace SimpleGL {

Input::Input(const std::weak_ptr<Window> &window): m_window(window) {
    const auto w = this->window();
    m_lastMouseX = static_cast<double>(w->screenWidth()) / 2.0;
    m_lastMouseY = static_cast<double>(w->screenHeight()) / 2.0;
}

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

void Input::cursorPosCallback(double xpos, double ypos) {
    if (m_firstMouse) {
        m_lastMouseX = xpos;
        m_lastMouseY = ypos;
        m_firstMouse = false;
    }

    m_mouseDelta.x = static_cast<float>(xpos - m_lastMouseX);
    m_mouseDelta.y = static_cast<float>(ypos - m_lastMouseY); // Corrected for natural y-axis movement

    m_lastMouseX = xpos;
    m_lastMouseY = ypos;
}

void Input::endFrame() {
    m_previousKeyStates = m_currentKeyStates;
    m_previousMouseStates = m_currentMouseStates;
    m_mouseDelta = {0.0f, 0.0f};

    updateDeltaTime();
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

void Input::setKeyState(int key, bool pressed) {
    m_currentKeyStates[key] = pressed;
}

void Input::updateDeltaTime() {
    m_deltaTime = Window::time() - m_lastFrameTime;
    m_lastFrameTime = Window::time();
}

}
