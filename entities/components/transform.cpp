#include "transform.h"

#include "rigid_body.h"
#include "../node.h"
#include "../scene.h"
#include "../../managers/engine.h"

namespace SimpleGL {

std::shared_ptr<Transform> Transform::getGlobal() {
    return Engine::get()->scene()->rootNode()->transform();
}

void Transform::setPosition(float x, float y, float z) {
    markAsDirty();
    m_position = glm::vec3(x, y, z);
}
void Transform::setPosition(const glm::vec3 position) {
    setPosition(position.x, position.y, position.z);
}

void Transform::setOrientation(float w, float x, float y, float z) {
    markAsDirty();
    m_orientation = glm::quat(w, x, y, z);
}
void Transform::setOrientation(const glm::quat orientation) {
    setOrientation(orientation.w, orientation.x, orientation.y, orientation.z);
}

void Transform::setScale(float s) {
    setScale(s, s, s);
}
void Transform::setScale(const glm::vec3 scale) {
    setScale(scale.x, scale.y, scale.z);
}
void Transform::setScale(float x, float y, float z) {
    markAsDirty();
    m_scale = glm::vec3(x, y, z);
}

void Transform::translate(const glm::vec3& vector) {
    const auto displacement = m_orientation * vector;
    const auto newPosition = m_position + displacement;

    setPosition(newPosition);
}

void Transform::rotate(const glm::quat& rotation, const std::shared_ptr<Transform>& transform) {
    if(transform != nullptr) {
        const glm::quat relativeRotation = (transform->m_orientation * rotation) * glm::conjugate(transform->m_orientation);

        const auto newOrientation = relativeRotation * m_orientation;
        setOrientation(newOrientation);
    }
    else {
        const auto newOrientation = m_orientation * rotation;
        setOrientation(newOrientation);
    }
}

void Transform::scaleBy(float x) {
    setScale(m_scale * x);
}

void Transform::recalculate() {
    const auto& rigidBody = node()->rigidBody();
    const auto& parentNode = node()->parent();
    const auto& parentTransform = parentNode->transform();

    if (rigidBody && !m_dirty && m_rigidBodyDirty) {
        glm::vec3 worldPosition;
        glm::quat worldOrientation;

        rigidBody->getWorldTransform(worldPosition, worldOrientation);

        m_position = worldPosition - parentTransform->m_absolutePosition;
        m_orientation = glm::inverse(parentTransform->m_absoluteOrientation) * worldOrientation;
    }

    if (m_dirty || m_rigidBodyDirty) {
        m_absolutePosition = parentTransform->m_absolutePosition + parentTransform->m_absoluteOrientation * m_position;
        m_absoluteScale = parentTransform->m_absoluteScale * m_scale;
        m_absoluteOrientation = glm::normalize(parentTransform->m_absoluteOrientation * m_orientation);

        m_transformMatrix = calculateTransformMatrix();
        m_direction = m_absoluteOrientation * glm::vec3(0, 0, 1);
    }

    if (rigidBody && m_dirty) {
        rigidBody->setWorldTransform(m_absolutePosition, m_absoluteOrientation);
    }

    if (m_subtreeDirty || m_dirty) {
        for (const auto& childNode : node()->children()) {
            const auto& childTransform = childNode->transform();

            childTransform->m_dirty |= (m_dirty || m_rigidBodyDirty);
            childTransform->recalculate();
        }
    }

    m_subtreeDirty = false;
    m_dirty = false;
    m_rigidBodyDirty = false;
}

void Transform::onUpdate() {
    const auto& rigidBody = node()->rigidBody();

    if (rigidBody && rigidBody->isDynamic() && rigidBody->isActive()) {
        markAsRigidBodyDirty();
    }
}

void Transform::markAsDirty() {
    m_dirty = true;
    markAsSubtreeDirty();
}

void Transform::markAsRigidBodyDirty() {
    m_rigidBodyDirty = true;
    markAsSubtreeDirty();
}

void Transform::markAsSubtreeDirty() {
    m_subtreeDirty = true;

    auto currentNode = node()->parent();

    while (currentNode) {
        if (currentNode->transform()->m_subtreeDirty) {
            break;
        }

        currentNode->transform()->m_subtreeDirty = true;
        currentNode = currentNode->parent();
    }
}

glm::mat4 Transform::calculateTransformMatrix() const {
    auto matrix = glm::mat4(1.0f);

    matrix = glm::translate(matrix, m_absolutePosition);
    matrix = matrix * mat4_cast(m_absoluteOrientation);
    matrix = glm::scale(matrix, m_absoluteScale);

    return matrix;
}

}
