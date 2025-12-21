#include "portal_bullet.h"

#include <BulletCollision/CollisionShapes/btSphereShape.h>

#include "../transform.h"
#include "../rigid_body.h"
#include "../../node.h"
#include "../../../managers/engine.h"
#include "../../../managers/physics_manager.h"
#include "../../physics/contact_callback.h"

namespace SimpleGL {

std::shared_ptr<PortalBullet> PortalBullet::create(const std::shared_ptr<Node> &node, const std::string &name) {
    auto instance = base_create<PortalBullet>(node, name);

    const auto shape = std::make_shared<btSphereShape>(0.1f);
    const auto rigidBody = RigidBody::create(instance->node(), "bulletRigidBody");
    rigidBody->setMass(1.f);
    rigidBody->setCollisionShape(shape);
    rigidBody->init();

    const auto btRigidBody = rigidBody->getBtRigidBody();
    btRigidBody->setGravity(btVector3(0, 0, 0));
    btRigidBody->setCollisionFlags(
        btRigidBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE
    );
    btRigidBody->setCcdMotionThreshold(0.01f);
    btRigidBody->setCcdSweptSphereRadius(0.05f);

    return instance;
}

void PortalBullet::onStart() {
    node()->visible = false;
}

void PortalBullet::onUpdate() {
    auto btRigidBody = node()->rigidBody()->getBtRigidBody();

    ContactCallback callback;
    Engine::instance().physicsManager()->dynamicsWorld()->contactTest(btRigidBody.get(), callback);

    if (callback.hasHit) {
        // node()->visible = false;

        btRigidBody->setLinearVelocity(btVector3(0, 0, 0));
    }
}

void PortalBullet::shoot(const std::shared_ptr<Transform> &origin, const btVector3 &direction) const {
    node()->visible = true;

    node()->rigidBody()->setWorldTransform(
        origin->absolutePosition(),
        origin->absoluteOrientation()
    );

    auto btRigidBody = node()->rigidBody()->getBtRigidBody();
    btRigidBody->activate(true);
    btRigidBody->clearForces();
    btRigidBody->setLinearVelocity(direction * m_bulletSpeed);
    btRigidBody->setAngularVelocity(btVector3(0, 0, 0));
}

}
