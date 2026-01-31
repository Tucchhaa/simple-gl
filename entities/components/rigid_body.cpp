#include "rigid_body.h"

#include <LinearMath/btDefaultMotionState.h>
#include <BulletDynamics/Dynamics/btDynamicsWorld.h>

#include "transform.h"
#include "../node.h"
#include "../../managers/engine.h"
#include "../../helpers/converter.h"
#include "../../helpers/quick_accessors.h"

namespace SimpleGL {

RigidBody::~RigidBody() {
    dynamicsWorld()->removeRigidBody(m_rigidBody.get());

    m_motionState.reset();
    m_rigidBody.reset();
    m_collisionShape.reset();
}

void RigidBody::attachTo(const std::shared_ptr<Node> &node) {
    Component::attachTo(node);
    node->m_rigidBody = std::static_pointer_cast<RigidBody>(shared_from_this());
}

void RigidBody::getWorldTransform(glm::vec3& position, glm::quat& rotation) const {
    btTransform worldTransform;
    m_motionState->getWorldTransform(worldTransform);

    position = Converter::toGlm(worldTransform.getOrigin());
    rotation = Converter::toGlm(worldTransform.getRotation());
}

void RigidBody::setWorldTransform(const glm::vec3 &position, const glm::quat& rotation) const {
    btTransform newTransform;
    newTransform.setIdentity();
    newTransform.setOrigin(Converter::toBt(position));
    newTransform.setRotation(Converter::toBt(rotation));

    m_rigidBody->setWorldTransform(newTransform);
    m_motionState->setWorldTransform(newTransform);

    m_rigidBody->setLinearVelocity(btVector3(0, 0, 0));
    m_rigidBody->setAngularVelocity(btVector3(0, 0, 0));

    m_rigidBody->activate(true);
}

void RigidBody::init() {
    if (m_collisionShape == nullptr) {
        return;
    }

    btVector3 startPosition = Converter::toBt(transform()->position());

    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(startPosition);

    btVector3 localInertia;

    if (isDynamic()) {
        m_collisionShape->calculateLocalInertia(m_mass, localInertia);
    }

    m_motionState = std::make_shared<btDefaultMotionState>(startTransform);
    btRigidBody::btRigidBodyConstructionInfo info(
        m_mass,
        m_motionState.get(),
        m_collisionShape.get(),
        localInertia
    );
    m_rigidBody = std::make_shared<btRigidBody>(info);

    dynamicsWorld()->addRigidBody(m_rigidBody.get(), group, mask);
}

void RigidBody::reinit() const {
    dynamicsWorld()->removeRigidBody(m_rigidBody.get());
    dynamicsWorld()->addRigidBody(m_rigidBody.get(), group, mask);
}

}
