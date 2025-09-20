#include "camera.h"

#include "transform.h"
#include "../../managers/engine.h"
#include "../../managers/window_manager.h"

namespace SimpleGL {

void Camera::recalculate() {
    m_viewMatrix = glm::mat4(1.0f);

    m_viewMatrix = m_viewMatrix * glm::mat4_cast(glm::conjugate(transform()->absoluteOrientation()));
    m_viewMatrix = translate(m_viewMatrix, -transform()->absolutePosition());
}

glm::mat4 Camera::calculateProjectionMatrix(float fov, float near, float far) {
    const float aspect = Engine::instance().mainWindow()->aspectRatio();

    return glm::perspective(fov, aspect, near, far);
}

}
