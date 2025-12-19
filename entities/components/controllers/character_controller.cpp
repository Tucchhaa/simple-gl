#include "character_controller.h"

#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>

#include "../camera.h"
#include "../rigid_body.h"
#include "../transform.h"
#include "../../window.h"
#include "../../node.h"
#include "../../../managers/engine.h"
#include "../../../entities/input.h"
#include "../../../helpers/converter.h"
#include "../../../managers/physics_manager.h"

namespace SimpleGL {

void CharacterController::onStart() {
    if (node()->rigidBody() == nullptr) {
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
    if (this->canJump() && input->isKeyPressed(GLFW_KEY_SPACE)) {
        rigidBody->applyCentralForce(btVector3(0, 25000, 0));
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
        Engine::instance().mainWindow()->isCursorPositionFixed = m_canRotate;
    }
}

bool CharacterController::canJump() const {
    auto physicsWorld = Engine::instance().physicsManager()->dynamicsWorld();
    auto rigidBody = node()->rigidBody()->getBtRigidBody().get();
    int numManifolds = physicsWorld->getDispatcher()->getNumManifolds();

    for (int i = 0; i < numManifolds; i++) {
        btPersistentManifold* manifold = physicsWorld->getDispatcher()->getManifoldByIndexInternal(i);

        if (manifold->getBody0() != rigidBody && manifold->getBody1() != rigidBody)
            continue;

        for (int j = 0; j < manifold->getNumContacts(); j++) {
            const btManifoldPoint& point = manifold->getContactPoint(j);

            if (point.getDistance() < 0.15f) {
                btVector3 normal = point.m_normalWorldOnB;
                if (manifold->getBody1() == rigidBody)
                    normal = -normal;

                if (normal.dot(btVector3(0, 1, 0)) > 0.6f) {
                    return true;
                }
            }
        }
    }

    return false;
}

}
