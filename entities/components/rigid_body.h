#pragma once

#include <glm/glm.hpp>

#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btMotionState.h>

#include "component.h"

class btDynamicsWorld;

namespace SimpleGL {

class RigidBody : public Component {
public:
    class Factory : public ComponentFactory<RigidBody> {};

    int group = -1; // 0xffffffff
    int mask = -1; // 0xffffffff

    explicit RigidBody(const std::string &name = "RigidBody"): Component(name) {}

    ~RigidBody() override;

    void attachTo(const std::shared_ptr<Node> &node) override;

    const std::shared_ptr<btRigidBody>& getBtRigidBody() const { return m_rigidBody; }

    void setMass(float mass) { m_mass = mass; }

    void setCollisionShape(const std::shared_ptr<btCollisionShape> &collisionShape) {
        m_collisionShape = collisionShape;
    }

    bool isDynamic() const { return m_mass != 0.0f; }

    void getWorldTransform(glm::vec3& position, glm::quat& rotation) const;

    void setWorldTransform(const glm::vec3& position, const glm::quat& rotation) const;

    void init();

    void reinit() const;

private:
    std::shared_ptr<btCollisionShape> m_collisionShape;
    std::shared_ptr<btMotionState> m_motionState;
    std::shared_ptr<btRigidBody> m_rigidBody;

    float m_mass = 0.0f;

    // static inline const std::unique_ptr<btDynamicsWorld>& dynamicsWorld();
};

}
