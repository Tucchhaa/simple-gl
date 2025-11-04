#pragma once

#include <memory>
#include <array>

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

namespace SimpleGL {

class Window;

class Input {
public:
    static std::shared_ptr<Input> create(const std::weak_ptr<Window>& window);

    void process();

    glm::vec2 axisVec2() const;

    glm::vec2 mouseDelta() const;

    bool isKeyDown(int key) const;
    bool isKeyPressed(int key) const;
    bool isKeyReleased(int key) const;

    bool isMouseButtonDown(int button) const;
    bool isMouseButtonPressed(int button) const;
    bool isMouseButtonReleased(int button) const;

    float deltaTime() const { return m_deltaTime; }

private:
    std::weak_ptr<Window> m_window;

    float m_lastFrameTime = 0;
    float m_deltaTime = 0;

    float m_yPositivePressed = 0;
    float m_yNegativePressed = 0;
    float m_xPositivePressed = 0;
    float m_xNegativePressed = 0;

    double m_mouseX = 0;
    double m_mouseY = 0;

    std::array<bool, GLFW_KEY_LAST + 1> m_currentKeyStates{};
    std::array<bool, GLFW_KEY_LAST + 1> m_previousKeyStates{};

    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_currentMouseStates{};
    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_previousMouseStates{};

    explicit Input(const std::weak_ptr<Window> &window): m_window(window) {};

    void reset();

    void updateCursorPosition();

    void updateDeltaTime();

    std::shared_ptr<Window> window() const { return m_window.lock(); }
};

}
