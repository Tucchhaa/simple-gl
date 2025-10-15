#include "transform.h"

#include "../scene.h"
#include "../../managers/engine.h"

namespace SimpleGL {

std::shared_ptr<Transform> Transform::getGlobal() {
    return Engine::instance().scene()->rootNode()->transform();
}

void Transform::setPosition(float x, float y, float z) { m_position = glm::vec3(x, y, z); }
void Transform::setPosition(const glm::vec3 position) { m_position = position; }
void Transform::setOrientation(float w, float x, float y, float z) { m_orientation = glm::quat(w, x, y, z); }
void Transform::setOrientation(const glm::quat orientation) { m_orientation = orientation; }
void Transform::setScale(float s) { m_scale = glm::vec3(s); }
void Transform::setScale(float x, float y, float z) { m_scale = glm::vec3(x, y, z); }
void Transform::setScale(const glm::vec3 scale) { m_scale = scale; }

void Transform::translate(const glm::vec3& vector) {
    m_position += m_orientation * vector;
}

void Transform::rotate(const glm::quat& rotation, const std::shared_ptr<Transform>& transform) {
    if(transform != nullptr) {
        const glm::quat relativeRotation = (transform->m_orientation * rotation) * glm::conjugate(transform->m_orientation);

        m_orientation = relativeRotation * m_orientation;
    }
    else
        m_orientation *= rotation;
}

void Transform::recalculate() {
    const auto parentNode = m_node.lock();
    const auto parentTransform = parentNode->parent()->transform();

    m_absolutePosition = parentTransform->m_absolutePosition + m_position;
    m_absoluteScale = parentTransform->m_absoluteScale * m_scale;
    m_absoluteOrientation = glm::normalize(parentTransform->m_absoluteOrientation * m_orientation);

    m_transformMatrix = calculateTransformMatrix();
    m_direction = m_absoluteOrientation * glm::vec3(0, 0, 1);
}

glm::mat4 Transform::calculateTransformMatrix() const {
    auto matrix = glm::mat4(1.0f);

    matrix = glm::translate(matrix, m_absolutePosition);
    matrix = matrix * mat4_cast(m_absoluteOrientation);
    matrix = glm::scale(matrix, m_absoluteScale);

    return matrix;
}

}
