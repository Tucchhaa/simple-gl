#include "character_controller.h"

#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>

#include "../camera.h"
#include "../rigid_body.h"
#include "../transform.h"
#include "../../physics/contact_callback.h"
#include "../../window.h"
#include "../../node.h"
#include "../../../managers/engine.h"
#include "../../../entities/input.h"
#include "../../../helpers/converter.h"
#include "../../../managers/physics_manager.h"

namespace SimpleGL {

void CharacterController::onStart() {
    if (m_rigidBody == nullptr) {
        throw std::runtime_error("Character Controller: RigidBody is not set");
    }

    if (m_cameraNode == nullptr) {
        throw std::runtime_error("Character Controller: Camera is not set");
    }

    const auto rigidBody = node()->rigidBody()->getBtRigidBody();
    rigidBody->setAngularFactor(btVector3(0, 0, 0));
    rigidBody->setFriction(0);
    rigidBody->setActivationState(DISABLE_DEACTIVATION);
}

void CharacterController::onUpdate() {
    const auto input = Engine::instance().mainWindow()->input();
    const auto rigidBody = node()->rigidBody()->getBtRigidBody();
    const auto cameraTransform = m_cameraNode->transform();

    // Displacement
    const auto axis = input->axisVec2();

    if (axis.x == 0 && axis.y == 0) {
        const auto velocity = btVector3(0, rigidBody->getLinearVelocity().y(), 0);
        rigidBody->setLinearVelocity(velocity);
    }
    else {
        const auto proj = glm::normalize(glm::vec3(
            cameraTransform->direction().x,
            0,
            cameraTransform->direction().z
        ));
        const auto perp = glm::normalize(glm::cross(glm::vec3(0, 1, 0), proj));

        const auto direction = glm::normalize(-proj * axis.y + perp * axis.x);
        const auto velocity = btVector3(
            direction.x * speed * input->deltaTime(),
            rigidBody->getLinearVelocity().y(),
            direction.z * speed * input->deltaTime()
        );

        rigidBody->setLinearVelocity(velocity);
    }

    // Jump
    if (m_jumpReloadLeftMs > 0) {
        m_jumpReloadLeftMs -= input->deltaTime() * 1000;
    }

    if (this->isTouchingGround() && input->isKeyDown(GLFW_KEY_SPACE) && m_jumpReloadLeftMs <= 0) {
        rigidBody->applyCentralImpulse(btVector3(0, 400, 0));
        m_jumpReloadLeftMs = m_jumpReloadTimeMs;
    }

    // Camera Orientation
    if (m_canRotate) {
        float pitchDelta = input->mouseDelta().y * rotationSpeed;
        float yawDelta = input->mouseDelta().x * rotationSpeed;

        float newPitch = glm::clamp(m_pitch + pitchDelta, -m_maxPitch, m_maxPitch);
        float clampedPitchDelta = newPitch - m_pitch;

        m_pitch = newPitch;

        const glm::quat qPitch = glm::angleAxis(clampedPitchDelta, glm::vec3(-1, 0, 0));
        const glm::quat qYaw = glm::angleAxis(yawDelta, glm::vec3(0, -1, 0));

        cameraTransform->rotate(qYaw, Transform::getGlobal());
        cameraTransform->rotate(qPitch);
    }

    if (input->isKeyPressed(GLFW_KEY_ENTER)) {
        m_canRotate = !m_canRotate;
        auto window = Engine::instance().mainWindow();
        window->isCursorPositionFixed = m_canRotate;

        if (m_canRotate) {
            glfwSetInputMode(window->glfwWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            window->setCursorPositionToCenter();
        } else {
            glfwSetInputMode(window->glfwWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

bool CharacterController::isTouchingGround() const {
    auto physicsWorld = Engine::instance().physicsManager()->dynamicsWorld();
    auto rigidBody = node()->rigidBody()->getBtRigidBody();

    ContactCallback callback;
    Engine::instance().physicsManager()->dynamicsWorld()->contactTest(rigidBody.get(), callback);

    for (const auto& contactResult : callback.results) {
        const btManifoldPoint& point = contactResult.hitPoint;
        const btVector3& normal = point.m_normalWorldOnB;

        if (point.getDistance() <= 0.05f && normal.dot(btVector3(0, 1, 0)) > 0.6f) {
            return true;
        }
    }

    return false;
}

}
