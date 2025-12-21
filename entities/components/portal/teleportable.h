#pragma once

#include <glm/fwd.hpp>

#include "../component.h"

namespace SimpleGL {

class Portal;
class RigidBody;
class MeshComponent;
class Camera;

class Teleportable : public Component {
public:
    Teleportable(
        const std::weak_ptr<Node>& node,
        const std::string &name
    ): Component(node, name) {}

    static std::shared_ptr<Teleportable> create(
        const std::shared_ptr<Node>& node,
        const std::string& name = "Teleportable"
    ) {
        auto instance = base_create<Teleportable>(node, name);
        return instance;
    }

    void setPortal(const std::shared_ptr<Portal>& portal) { m_portal = portal; }
    void setRigidBody(const std::shared_ptr<RigidBody>& rigidBody) { m_rigidBody = rigidBody; }
    void setMeshes(const std::vector<std::shared_ptr<MeshComponent>>& meshes) { m_meshes = meshes; }
    void setAllowPortalGroup(int teleportableGroup) { m_allowPortalGroup = teleportableGroup; }

    void onStart() override;

    void onUpdate() override;

    void draw(const std::shared_ptr<Camera>& camera) const;

private:
    std::shared_ptr<Portal> m_portal;

    std::shared_ptr<RigidBody> m_rigidBody;

    std::vector<std::shared_ptr<MeshComponent>> m_meshes;

    int m_allowPortalGroup = -1;

    float m_thresholdDistance2 = 1.f;

    bool m_disabledPortal1Collision = false;
    bool m_disabledPortal2Collision = false;


    void toggleCollisionIfNeed(
        const std::shared_ptr<Node>& portalNode,
        bool& disabledPortalCollision
    ) const;

    void teleportIfNeed(
        const std::shared_ptr<Node>& sourcePortalNode,
        const std::shared_ptr<Node> &destPortalNode
    ) const;

    void drawClone(
        const std::shared_ptr<Camera>& camera,
        const std::shared_ptr<Node>& sourcePortalNode,
        const std::shared_ptr<Node> &destPortalNode
    ) const;

    void getTeleportedTransform(
        const std::shared_ptr<Node>& sourcePortalNode,
        const std::shared_ptr<Node> &destPortalNode,
        glm::quat& qNew,
        glm::vec3& pNew,
        glm::quat& qDelta
    ) const;
};

}
