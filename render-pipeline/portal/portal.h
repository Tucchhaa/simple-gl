#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <glm/fwd.hpp>

namespace SimpleGL {

class Node;
class Camera;
class Transform;
class MeshData;
class MeshComponent;

class Portal {
public:
    std::shared_ptr<Node> portal1Node;
    std::shared_ptr<Node> portal2Node;

    explicit Portal(const std::shared_ptr<Camera>& camera);

    static std::shared_ptr<Portal> create(
        const std::shared_ptr<Camera>& camera
    );

    void drawPortalContents(
        int portalIndex,
        std::function<void(const std::shared_ptr<Camera>& camera)> drawScene
    ) const;

private:
    int m_maxRecursionLevel = 2;

    std::shared_ptr<Camera> m_camera;

    std::shared_ptr<Node> m_virtualCamerasNode;
    std::vector<std::shared_ptr<Camera>> m_virtualCameras;

    void createVirtualCameras();

    std::vector<std::shared_ptr<Camera>> getRecursiveCameras(
        const std::shared_ptr<Node>& sourcePortal,
        const std::shared_ptr<Node>& destPortal
    ) const;

    std::pair<glm::quat, glm::vec3> calcVirtualCameraTransform(
        const std::shared_ptr<Transform>& sourceT,
        const std::shared_ptr<Transform>& destT
    ) const;
};

}
