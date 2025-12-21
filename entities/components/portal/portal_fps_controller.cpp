#include "./portal_fps_controller.h"

#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>

#include "./portal_bullet.h"
#include "../transform.h"
#include "../../input.h"
#include "../../node.h"
#include "../../window.h"
#include "../../../helpers/converter.h"
#include "../../../managers/engine.h"
#include "../../../managers/physics_manager.h"

namespace SimpleGL {

void PortalFPSController::onStart() {
    CharacterController::onStart();

    if (m_weaponNode == nullptr) {
        throw std::runtime_error("Portal FPS Controller: Weapon Node is not set");
    }

    if (m_portal1Bullet == nullptr || m_portal2Bullet == nullptr) {
        throw std::runtime_error("Portal FPS Controller: Portal Bullet is not set");
    }
}

void PortalFPSController::onUpdate() {
    CharacterController::onUpdate();

    const auto input = Engine::instance().mainWindow()->input();

    const bool isLMB = input->isMouseButtonPressed(GLFW_MOUSE_BUTTON_1);
    const bool isRMB = input->isMouseButtonPressed(GLFW_MOUSE_BUTTON_2);

    if (m_reloadLeftMs > 0) {
        m_reloadLeftMs -= input->deltaTime() * 1000;
    }

    if ((isLMB || isRMB) && m_reloadLeftMs <= 0) {
        const auto bullet = isLMB ? m_portal1Bullet : m_portal2Bullet;
        const auto shootDirection = getBulletDirection();

        bullet->shoot(m_weaponNode->transform(), shootDirection);

        m_reloadLeftMs = m_reloadTimeMs;
    }
}

btVector3 PortalFPSController::getBulletDirection() const {
    const auto origin = m_cameraNode->transform()->position();
    const auto cameraDirection = -m_cameraNode->transform()->direction();
    constexpr float maxDistance = 1000.f;

    const btVector3 from = Converter::toBt(origin);
    const btVector3 to = Converter::toBt(origin + cameraDirection * maxDistance);

    btCollisionWorld::ClosestRayResultCallback rayCast(from, to);

    Engine::instance().physicsManager()->dynamicsWorld()->rayTest(from, to, rayCast);

    if (rayCast.hasHit()) {
        const auto weaponPosition = Converter::toBt(m_weaponNode->transform()->position());
        return (rayCast.m_hitPointWorld - weaponPosition).normalize();
    }

    return Converter::toBt(-m_weaponNode->transform()->direction());
}

}
