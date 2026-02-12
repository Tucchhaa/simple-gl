#pragma once

#include <vector>
#include <glm/fwd.hpp>

#include "../component.h"

namespace SimpleGL {

class Portal;
class RigidBody;
class MeshComponent;
class Camera;

class Teleportable : public Component {
public:
    class Factory : public ComponentFactory<Teleportable> {};

    explicit Teleportable(const std::string& name = "Teleportable"): Component(name) {}

    void setPortal(const std::shared_ptr<Portal>& portal) { m_portal = portal; }
    void setMeshes(const std::vector<std::shared_ptr<MeshComponent>>& meshes) { m_meshes = meshes; }
    void setAllowPortalGroup(int teleportableGroup) { m_allowPortalGroup = teleportableGroup; }

    void onStart() override;
    void onUpdate() override;

    void draw(const std::shared_ptr<Camera>& camera) const;

private:
    std::shared_ptr<Portal> m_portal;
    std::vector<std::shared_ptr<MeshComponent>> m_meshes;

    int m_allowPortalGroup = -1;
    float m_thresholdDistance2 = 1.5f;
    bool m_disabledPortalCollision = false;

    bool m_isCloseEnough1 = false;
    bool m_isCloseEnough2 = false;

    void toggleCollisionIfNeed();

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
