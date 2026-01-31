#pragma once

#include <glm/vec3.hpp>
#include <LinearMath/btTransform.h>
#include <LinearMath/btVector3.h>

#include "../component.h"

namespace SimpleGL {

class ContactResult;
class RigidBody;

class PortalBullet : public Component {
public:
    class Factory : public ComponentFactory<PortalBullet> {};

    explicit PortalBullet(const std::string &name = "PortalBullet"): Component(name) {}

    void setPortalNode(const std::shared_ptr<Node>& node) { m_portalNode = node; }
    void setRigidBody(const std::shared_ptr<RigidBody>& rigidBody) { m_rigidBody = rigidBody; }

    void onStart() override;
    void onUpdate() override;

    void placePortal(glm::vec3 position, glm::vec3 normal) const;

    void shoot(const std::shared_ptr<Transform>& origin, const btVector3& direction) const;

private:
    float m_bulletSpeed = 30.f;

    std::shared_ptr<Node> m_portalNode;
    std::shared_ptr<RigidBody> m_rigidBody;

    btTransform m_previousTransform;

    bool computeImpactPoint(glm::vec3& position, glm::vec3& normal) const;
};

}
