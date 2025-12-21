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
class PortalFramebuffer;
class ShaderProgram;

class Portal {
public:
    std::shared_ptr<Node> portal1Node;
    std::shared_ptr<Node> portal2Node;

    explicit Portal(const std::shared_ptr<Camera>& camera);

    static std::shared_ptr<Portal> create(
        const std::shared_ptr<Camera>& camera
    );

    void setPortalMesh(
        int portalIndex,
        const std::shared_ptr<MeshData>& meshData
    );

    void drawPortal(
        int portalIndex,
        const std::function<void(const std::shared_ptr<Camera>& camera)>& drawScene
    );

    static std::pair<glm::quat, glm::vec3> calculatePortalTransform(
        const std::shared_ptr<Transform>& sourceT,
        const std::shared_ptr<Transform>& destT
    );

private:
    std::shared_ptr<PortalFramebuffer> m_tailPortalFramebuffer;

    ///This shader is used to render portal into stencil buffer
    std::shared_ptr<ShaderProgram> m_basicPortalShader;

    /// This shader is used to render tail portals
    std::shared_ptr<ShaderProgram> m_tailPortalShader;

    /// Number of recursive portals that rerender the whole scene to draw the portal contents.
    unsigned int m_maxRecursionLevel = 2;

    /// Number of recursive portal that use tailPortalFramebuffer as a texture to render portal contents.
    unsigned int m_maxTailRecursionLevel = 10;

    unsigned int getTotalRecursionLevel() const {
        return m_maxRecursionLevel + m_maxTailRecursionLevel;
    }

    std::shared_ptr<Camera> m_camera;
    std::shared_ptr<Camera> m_tailVirtualCamera;

    std::vector<std::shared_ptr<Camera>> m_virtualCameras;

    void createShaders();

    void createVirtualCameras();

    std::vector<std::shared_ptr<Camera>> getRecursiveCameras(
        const std::shared_ptr<Node>& sourcePortal,
        const std::shared_ptr<Node>& destPortal
    ) const;

    void drawTailPortalToFramebuffer(
        const std::function<void(const std::shared_ptr<Camera>& camera)>& drawScene
    ) const;
};

}
