#include "rigid_body.h"

#include <LinearMath/btDefaultMotionState.h>

#include "transform.h"
#include "../node.h"
#include "../../managers/engine.h"
#include "../../managers/physics_manager.h"
#include "../../helpers/converter.h"

namespace SimpleGL {

RigidBody::~RigidBody() {
    Engine::instance().physicsManager()->dynamicsWorld()->removeRigidBody(m_rigidBody.get());

    m_motionState.reset();
    m_rigidBody.reset();
    m_collisionShape.reset();
}

std::shared_ptr<RigidBody> RigidBody::create(const std::shared_ptr<Node> &node, const std::string &name) {
    auto instance = base_create<RigidBody>(node, name);
    node->m_rigidBody = instance;
    return instance;
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

    Engine::instance().physicsManager()->dynamicsWorld()->addRigidBody(m_rigidBody.get());
}

}
