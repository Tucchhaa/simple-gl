#include "free_controller.h"

#include "transform.h"
#include "../window.h"
#include "../../managers/engine.h"

namespace SimpleGL {

void FreeController::handleInput() const {
    auto input = Engine::instance().mainWindow()->input();

    auto axis = input->axisVec2() * speed * input->deltaTime();
    auto displacement = transform()->orientation() * glm::vec3(axis.x, 0, axis.y);

    transform()->translate(displacement);
}

}
