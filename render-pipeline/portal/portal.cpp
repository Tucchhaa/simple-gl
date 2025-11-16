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
}

void Portal::drawPortalContents(
    int portalIndex,
    const std::function<void(const std::shared_ptr<Camera>& camera)>& drawScene
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
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

    for (int i=0; i < m_maxRecursionLevel; i++) {
        glStencilFunc(GL_EQUAL, i, 0xFF);

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

        // draw portal border
        glStencilFunc(GL_EQUAL, i - 1, 0xFF);

        const glm::vec3 originalScale = portalMesh->transform()->scale();
        portalMesh->transform()->scaleBy(1.05f);
        portalMesh->transform()->recalculate();

        portalMesh->draw(recursiveCameras[i - 1]);

        glDepthFunc(GL_LESS);

        // draw portal mesh to z-buffer
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glStencilFunc(GL_LEQUAL, i, 0xFF);
        glDepthFunc(GL_ALWAYS);
        glDepthMask(GL_TRUE);

        portalMesh->draw(recursiveCameras[i - 1]);

        glDepthFunc(GL_LESS);

        portalMesh->transform()->setScale(originalScale);
        portalMesh->transform()->recalculate();
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
    result.reserve(m_maxRecursionLevel + 1);

    result.push_back(m_camera);

    auto pPrev = m_camera->transform()->absolutePosition();
    auto qPrev = m_camera->transform()->absoluteOrientation();

    auto [
        qDelta, pDelta
    ] = calcVirtualCameraTransform(sourcePortal->transform(), destPortal->transform());

    for (int i = 0; i < m_maxRecursionLevel; i++) {
        const auto virtualCamera = m_virtualCameras[i];

        const auto pNew = pDelta + (qDelta * pPrev);
        const auto qNew = qDelta * qPrev;

        virtualCamera->transform()->setPosition(pNew);
        virtualCamera->transform()->setOrientation(qNew);
        virtualCamera->transform()->recalculate();

        virtualCamera->recalculateViewMatrix();
        virtualCamera->setNearPlane(destPortal->transform());

        result.push_back(virtualCamera);
        pPrev = pNew;
        qPrev = qNew;
    }

    return result;
}

std::pair<glm::quat, glm::vec3> Portal::calcVirtualCameraTransform(
    const std::shared_ptr<Transform>& sourceT,
    const std::shared_ptr<Transform>& destT
) {
    static auto flipY = glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 1, 0));

    const auto sQ = sourceT->absoluteOrientation();
    const auto dQ = destT->absoluteOrientation();
    const auto sP = sourceT->absolutePosition();
    const auto dP = destT->absolutePosition();

    const glm::quat qDelta = dQ * flipY * glm::inverse(sQ);
    const glm::vec3 pDelta = dP - (qDelta * sP);

    return { qDelta, pDelta };
}

}
