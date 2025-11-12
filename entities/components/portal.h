#pragma once

#include <memory>

#include "component.h"

namespace SimpleGL {

class Node;
class Camera;
class Transform;
class MeshData;

class Portal : public Component {
public:
    std::shared_ptr<Node> portal1Node;
    std::shared_ptr<Node> portal2Node;

    std::shared_ptr<Camera> portal1VirtualCamera;
    std::shared_ptr<Camera> portal2VirtualCamera;

    Portal(const std::weak_ptr<Node> &node, const std::string &name): Component(node, name) {}

    static std::shared_ptr<Portal> create(
        const std::shared_ptr<Node>& node,
        const std::shared_ptr<Camera>& camera,
        const std::string& name = "Portal"
    );

    void onUpdate() override;

    void setPortalMesh(const std::shared_ptr<MeshData>& meshData) const;

private:
    std::shared_ptr<Camera> m_camera;

    std::shared_ptr<Node> m_portal1VirtualCameraNode;
    std::shared_ptr<Node> m_portal2VirtualCameraNode;

    void createVirtualCamera(
        const std::string& name,
        std::shared_ptr<Node>& node,
        std::shared_ptr<Camera>& virtualCamera
    );

    void updateVirtualCameraTransform(
        const std::shared_ptr<Node>& sourcePortal,
        const std::shared_ptr<Node>& destPortal,
        const std::shared_ptr<Camera>& virtualCamera
    );
};

}
