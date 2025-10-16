#include "free_controller.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "transform.h"
#include "../window.h"
#include "../../managers/engine.h"
#include "../../entities/input.h"

namespace SimpleGL {

void FreeController::handleInput() {
    const auto input = Engine::instance().mainWindow()->input();

    const auto axis = input->axisVec2() * speed * input->deltaTime();
    const auto displacement = glm::vec3(axis.x, 0, -axis.y);

    transform()->translate(displacement);

    if (m_canRotate) {
        const glm::quat qPitch = glm::angleAxis(input->mouseDelta().y * rotationSpeed, glm::vec3(-1, 0, 0));
        const glm::quat qYaw = glm::angleAxis(input->mouseDelta().x * rotationSpeed, glm::vec3(0, -1, 0));

        transform()->rotate(qYaw, Transform::getGlobal());
        transform()->rotate(qPitch);
    }

    if (input->isKeyPressed(GLFW_KEY_SPACE)) {
        m_canRotate = !m_canRotate;
        Engine::instance().mainWindow()->isCursorPositionFixed = m_canRotate;
    }
}

}
