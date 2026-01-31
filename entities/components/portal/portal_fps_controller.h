#pragma once

#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>

#include "../controllers/character_controller.h"

namespace SimpleGL {

class PortalBullet;

class PortalFPSController : public CharacterController {
public:
    class Factory : public ComponentFactory<PortalFPSController> {};

    explicit PortalFPSController(const std::string &name = "FPSController"): CharacterController(name) {}

    void setWeaponNode(const std::shared_ptr<Node>& node) { m_weaponNode = node; }
    void setPortal1Bullet(const std::shared_ptr<PortalBullet>& bullet) { m_portal1Bullet = bullet; }
    void setPortal2Bullet(const std::shared_ptr<PortalBullet>& bullet) { m_portal2Bullet = bullet; }

    void onStart() override;
    void onUpdate() override;

private:
    std::shared_ptr<Node> m_weaponNode;
    std::shared_ptr<PortalBullet> m_portal1Bullet;
    std::shared_ptr<PortalBullet> m_portal2Bullet;

    const float m_reloadTimeMs = 750.f;
    float m_reloadLeftMs = 0.f;

    btVector3 getBulletDirection() const;
};

}
