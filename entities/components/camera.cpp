#include "camera.h"

#include "transform.h"
#include "../../managers/engine.h"
#include "../../entities/window.h"

namespace SimpleGL {

void Camera::recalculateViewMatrix() {
    m_viewMatrix = glm::mat4_cast(glm::conjugate(transform()->absoluteOrientation()));
    m_viewMatrix = glm::translate(m_viewMatrix, -transform()->absolutePosition());
}

void Camera::recalculateProjectionMatrix() {
    const float aspect = Engine::get()->window()->aspectRatio();

    m_projectionMatrix = glm::perspective(m_fov, aspect, m_near, m_far);
}

// Lengyel, Eric. "Oblique View Frustum Depth Projection and Clipping".
// Journal of Game Development, Vol. 1, No. 2 (2005)
// http://www.terathon.com/code/oblique.html
void Camera::setNearPlane(const std::shared_ptr<Transform> &planeTransform) {
    recalculateProjectionMatrix();

    glm::mat4 projection = projectionMatrix();

    auto normalViewMatrix = calculateViewNormalMatrix();

    auto portalNormal = planeTransform->direction();
    auto portalPosition = planeTransform->position();
    float D = -glm::dot(portalNormal, portalPosition);

    auto clipPlane = glm::vec4(portalNormal, D);
    clipPlane = normalViewMatrix * clipPlane;

    if (clipPlane.w > 0.0f)
        clipPlane = -clipPlane;

    auto q = glm::vec4(
        (glm::sign(clipPlane.x) + projection[2][0]) / projection[0][0],
        (glm::sign(clipPlane.y) + projection[2][1]) / projection[1][1],
        -1.0f,
        (1.0f + projection[2][2]) / projection[3][2]
    );

    glm::vec4 c = clipPlane * (2.0f / (glm::dot(clipPlane, q)));

    projection[0][2] = c.x;
    projection[1][2] = c.y;
    projection[2][2] = c.z + 1.f;
    projection[3][2] = c.w;

    m_projectionMatrix = projection;
}

/// normalView = inverse(transpose(viewMatrix))
/// After substitution and simplification we will get:
/// viewNormal = conjugate(rotation) * transpose(translation)
glm::mat4 Camera::calculateViewNormalMatrix() const {
    // optimized version of:
    // return glm::inverse(glm::transpose(viewMatrix()));

    auto normalViewMatrix = viewMatrix();

    normalViewMatrix[3][0] = 0;
    normalViewMatrix[3][1] = 0;
    normalViewMatrix[3][2] = 0;
    normalViewMatrix[0][3] = transform()->position().x;
    normalViewMatrix[1][3] = transform()->position().y;
    normalViewMatrix[2][3] = transform()->position().z;

    return normalViewMatrix;
}

}
