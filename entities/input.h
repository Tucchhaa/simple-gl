#pragma once

#include <memory>
#include <array>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace SimpleGL {

class Window;

class Input {
public:
    static std::shared_ptr<Input> create();

    void keyCallback(int key, int action);
    void mouseButtonCallback(int button, int action);

    void updateState();

    glm::vec2 axisVec2() const;
    glm::vec2 mouseDelta() const;

    bool isKeyDown(int key) const;
    bool isKeyPressed(int key) const;
    bool isKeyReleased(int key) const;

    bool isMouseButtonDown(int button) const;
    bool isMouseButtonPressed(int button) const;
    bool isMouseButtonReleased(int button) const;

    float deltaTime() const { return m_deltaTime; }

    void setKeyState(int key, bool pressed);

private:
    float m_lastFrameTime = 0;
    float m_deltaTime = 0;

    double m_mouseX = 0;
    double m_mouseY = 0;

    std::array<bool, GLFW_KEY_LAST + 1> m_currentKeyStates{};
    std::array<bool, GLFW_KEY_LAST + 1> m_previousKeyStates{};

    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_currentMouseStates{};
    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_previousMouseStates{};

    Input() = default;

    void updateCursorPosition();

    void updateDeltaTime();

    static std::shared_ptr<Window> window();
};

}
