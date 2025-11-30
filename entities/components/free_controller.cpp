#include "free_controller.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "transform.h"
#include "../window.h"
#include "../../managers/engine.h"
#include "../../entities/input.h"

namespace SimpleGL {

void FreeController::onUpdate() {
    handleInput();
}

void FreeController::handleInput() {
    auto window = Engine::instance().mainWindow();
    auto input = window->input();

    const auto axis = input->axisVec2() * speed * input->deltaTime();
    const auto displacement = glm::vec3(axis.x, 0, -axis.y);

    transform()->translate(displacement);

    if (m_canRotate) {
        const float sensitivity = 0.005f;
        const glm::quat qPitch = glm::angleAxis(input->mouseDelta().y * sensitivity, glm::vec3(-1, 0, 0));
        const glm::quat qYaw = glm::angleAxis(input->mouseDelta().x * sensitivity, glm::vec3(0, -1, 0));

        transform()->rotate(qYaw, Transform::getGlobal());
        transform()->rotate(qPitch);
    }

    if (input->isKeyPressed(GLFW_KEY_SPACE)) {
        m_canRotate = !m_canRotate;
        auto glfwWindow = Engine::instance().mainWindow()->glfwWindow();
        glfwSetInputMode(glfwWindow, GLFW_CURSOR, m_canRotate ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}

}
