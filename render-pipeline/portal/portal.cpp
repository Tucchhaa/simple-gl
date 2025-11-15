#include "portal.h"

#include <glad/glad.h>

#include "../../managers/engine.h"
#include "../../entities/node.h"
#include "../../entities/scene.h"
#include "../../entities/components/camera.h"
#include "../../entities/components/mesh.h"
#include "../../entities/components/transform.h"

namespace SimpleGL {

Portal::Portal(const std::shared_ptr<Camera> &camera): m_camera(camera) {
    const auto rootNode = Engine::instance().scene()->rootNode();

    portal1Node = Node::create("portal1", rootNode);
    portal2Node = Node::create("portal2", rootNode);

    createVirtualCameras();
}

std::shared_ptr<Portal> Portal::create(
    const std::shared_ptr<Camera>& camera
) {
    auto instance = std::make_shared<Portal>(camera);
    return instance;

    // instance->m_camera = camera;
    //
    //
    // Node::create("meshNode", instance->portal1Node);
    // Node::create("meshNode", instance->portal2Node);
    //
    // instance->createVirtualCameras();
    //
    // return instance;
}

void Portal::drawPortalContents(
    int portalIndex,
    std::function<void(const std::shared_ptr<Camera>& camera)> drawScene
) const {
    if (portalIndex != 1 && portalIndex != 2) {
        throw std::runtime_error("Portal: incorrect portal index");
    }

    const auto sourcePortalNode = portalIndex == 1 ? portal1Node : portal2Node;
    const auto destPortalNode = portalIndex == 1 ? portal2Node : portal1Node;

    const auto portalMesh = sourcePortalNode->getChildComponent<MeshComponent>();

    const auto recursiveCameras = getRecursiveCameras(sourcePortalNode, destPortalNode);

    // prepare stencil buffer
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    for (int i=0; i < m_maxRecursionLevel; i++) {
        glStencilFunc(GL_ALWAYS, i+1, 0xFF);

        portalMesh->draw(recursiveCameras[i]);
    }

    // draw portal contents from deepest to shallowest
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glStencilMask(0x00);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    for (int i=m_maxRecursionLevel; i >= 1; i--) {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_EQUAL, i, 0xFF);

        drawScene(recursiveCameras[i]);

        // draw portal mesh to z-buffer
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDisable(GL_STENCIL_TEST);

        portalMesh->draw(recursiveCameras[i - 1]);
    }
}

void Portal::createVirtualCameras() {
    m_virtualCamerasNode = Node::create("portalVirtualCameras", Engine::instance().scene()->rootNode());

    for (int i=0; i < m_maxRecursionLevel; i++) {
        auto node = Node::create("virtualCameraNode"+std::to_string(i), m_virtualCamerasNode);
        auto virtualCamera = Camera::create(
            node,
            m_camera->fov(),
            m_camera->near(),
            m_camera->far()
        );

        m_virtualCameras.push_back(virtualCamera);
    }
}

std::vector<std::shared_ptr<Camera>> Portal::getRecursiveCameras(
    const std::shared_ptr<Node>& sourcePortal,
    const std::shared_ptr<Node>& destPortal
) const {
    std::vector<std::shared_ptr<Camera>> result;

    result.push_back(m_camera);
    auto prev = m_camera;

    auto [
        qChange, pChange
    ] = calcVirtualCameraTransform(sourcePortal->transform(), destPortal->transform());

    for (int i = 0; i < m_maxRecursionLevel; i++) {
        const auto virtualCamera = m_virtualCameras[i];

        const auto newPosition = pChange + prev->transform()->absolutePosition();
        const auto newOrientation = qChange * prev->transform()->absoluteOrientation();

        virtualCamera->transform()->setPosition(newPosition);
        virtualCamera->transform()->setOrientation(newOrientation);
        virtualCamera->transform()->recalculate();

        virtualCamera->recalculateViewMatrix();
        virtualCamera->setNearPlane(destPortal->transform());

        result.push_back(virtualCamera);
        prev = virtualCamera;
    }

    return result;
}

std::pair<glm::quat, glm::vec3> Portal::calcVirtualCameraTransform(
    const std::shared_ptr<Transform>& sourceT,
    const std::shared_ptr<Transform>& destT
) const {
    const auto cT = m_camera->transform();

    static auto flipY = glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 1, 0));

    const glm::quat sQInverse = glm::inverse(sourceT->absoluteOrientation());

    const glm::quat qRel = sQInverse * cT->absoluteOrientation();
    const glm::quat qVirtual = destT->absoluteOrientation() * flipY * qRel;
    const glm::quat qResult = qVirtual * glm::inverse(cT->absoluteOrientation());

    const glm::vec3 pRel = sQInverse * (cT->absolutePosition() - sourceT->absolutePosition());
    const glm::vec3 pVirtual = destT->absolutePosition() + destT->absoluteOrientation() * (flipY * pRel);
    const glm::vec3 pResult = pVirtual - cT->absolutePosition();

    return { qResult, pResult };
}

}
