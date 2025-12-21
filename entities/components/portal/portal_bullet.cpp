#include "portal_bullet.h"

#include <glm/gtc/quaternion.hpp>

#include <BulletCollision/CollisionShapes/btSphereShape.h>

#include "../transform.h"
#include "../rigid_body.h"
#include "../../node.h"
#include "../../../helpers/converter.h"
#include "../../../managers/engine.h"
#include "../../../managers/physics_manager.h"
#include "../../physics/contact_callback.h"
#include "../../physics/sweep_callback.h"

namespace SimpleGL {

std::shared_ptr<PortalBullet> PortalBullet::create(const std::shared_ptr<Node> &node, const std::string &name) {
    auto instance = base_create<PortalBullet>(node, name);

    const auto btRigidBody = instance->node()->rigidBody()->getBtRigidBody();
    btRigidBody->setGravity(btVector3(0, 0, 0));
    btRigidBody->setCcdMotionThreshold(0.01f);
    btRigidBody->setCcdSweptSphereRadius(0.05f);

    instance->m_previousTransform = btRigidBody->getWorldTransform();

    return instance;
}

void PortalBullet::onStart() {
    if (m_rigidBody == nullptr) {
        throw std::runtime_error("Portal Bullet: Rigid Body is not set");
    }

    if (m_portalNode == nullptr) {
        throw std::runtime_error("Portal Bullet: Portal Node is not set");
    }

    node()->visible = false;
}

void PortalBullet::onUpdate() {
    auto btRigidBody = node()->rigidBody()->getBtRigidBody();

    if (btRigidBody->getActivationState() == ACTIVE_TAG) {
        ContactCallback callback;
        Engine::instance().physicsManager()->dynamicsWorld()->contactTest(btRigidBody.get(), callback);

        if (callback.hasHit()) {
            glm::vec3 position, normal;
            bool foundImpactPoint = computeImpactPoint(position, normal);

            if (foundImpactPoint) {
                placePortal(position, normal);
            }

            node()->visible = false;
            btRigidBody->setActivationState(DISABLE_SIMULATION);
            btRigidBody->setLinearVelocity(btVector3(0, 0, 0));
        }
    }
}

void PortalBullet::placePortal(glm::vec3 position, glm::vec3 normal) {
    auto shiftedPosition = position + normal * 0.1f;
    auto orientation = glm::quatLookAt(-normal, glm::vec3(0, 1, 0));

    m_portalNode->transform()->setPosition(shiftedPosition);
    m_portalNode->transform()->setOrientation(orientation);
}

void PortalBullet::shoot(const std::shared_ptr<Transform> &origin, const btVector3 &direction) const {
    auto btRigidBody = node()->rigidBody()->getBtRigidBody();
    btRigidBody->forceActivationState(ACTIVE_TAG);
    btRigidBody->activate(true);

    node()->visible = true;
    node()->rigidBody()->setWorldTransform(
        origin->absolutePosition(),
        origin->absoluteOrientation()
    );

    btRigidBody->clearForces();
    btRigidBody->setLinearVelocity(direction * m_bulletSpeed);
    btRigidBody->setAngularVelocity(btVector3(0, 0, 0));
}

bool PortalBullet::computeImpactPoint(glm::vec3& position, glm::vec3& normal) const {
    const auto world = Engine::instance().physicsManager()->dynamicsWorld();
    const auto btRigidBody = node()->rigidBody()->getBtRigidBody();
    const btTransform& from = m_previousTransform;
    const btTransform& to = btRigidBody->getWorldTransform();

    const btSphereShape sphere = btSphereShape(0.2f);

    SweepCallback cb(from.getOrigin(), to.getOrigin(), btRigidBody.get());

    cb.m_collisionFilterGroup = btRigidBody->getBroadphaseProxy()->m_collisionFilterGroup;
    cb.m_collisionFilterMask = btRigidBody->getBroadphaseProxy()->m_collisionFilterMask;

    world->convexSweepTest(&sphere, from, to, cb);

    if (!cb.hasHit())
        return false;

    position = Converter::toGlm(cb.m_hitPointWorld);
    normal = Converter::toGlm(cb.m_hitNormalWorld);

    return true;
}

}
