#include "camera.h"

#include "transform.h"
#include "../../managers/engine.h"
#include "../../entities/window.h"

namespace SimpleGL {

void Camera::recalculateViewMatrix() {
    m_viewMatrix = glm::mat4(1.0f);

    m_viewMatrix = m_viewMatrix * glm::mat4_cast(glm::conjugate(transform()->absoluteOrientation()));
    m_viewMatrix = glm::translate(m_viewMatrix, -transform()->absolutePosition());
}

void Camera::recalculateProjectionMatrix(float fov, float near, float far) {
    const float aspect = Engine::instance().mainWindow()->aspectRatio();

    m_projectionMatrix = glm::perspective(fov, aspect, near, far);
}

}
