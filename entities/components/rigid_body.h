#pragma once

#include <glm/glm.hpp>

#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <glm/detail/type_quat.hpp>
#include <LinearMath/btMotionState.h>

#include "component.h"

namespace SimpleGL {

class RigidBody : public Component {
public:
    RigidBody(const std::weak_ptr<Node> &node, const std::string &name): Component(node, name) {}

    ~RigidBody() override;

    static std::shared_ptr<RigidBody> create(const std::shared_ptr<Node> &node, const std::string& name = "RigidBody");

    void setMass(float mass) { m_mass = mass; }

    void setCollisionShape(const std::shared_ptr<btCollisionShape> &collisionShape) {
        m_collisionShape = collisionShape;
    }

    bool isDynamic() const { return m_mass != 0.0f; }

    void getWorldTransform(glm::vec3& position, glm::quat& rotation) const;

    void setWorldTransform(const glm::vec3& position, const glm::quat& rotation) const;

    void onStart() override;

private:
    std::shared_ptr<btCollisionShape> m_collisionShape;
    std::shared_ptr<btMotionState> m_motionState;
    std::shared_ptr<btRigidBody> m_rigidBody;

    float m_mass = 0.0f;
};

}
