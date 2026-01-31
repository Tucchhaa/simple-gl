#include "portal.h"

#include <glad/glad.h>

#include "portal_framebuffer.h"
#include "../../managers/engine.h"
#include "../../managers/shader_manager.h"
#include "../../entities/node.h"
#include "../../entities/scene.h"
#include "../../entities/shader_program.h"
#include "../../entities/components/camera.h"
#include "../../entities/components/mesh.h"
#include "../../entities/components/transform.h"

namespace SimpleGL {

Portal::Portal(const std::shared_ptr<Camera> &camera): m_camera(camera) {
    m_tailPortalFramebuffer = std::make_shared<PortalFramebuffer>(1200 * 2, 900 * 2); // TODO

    const auto rootNode = Engine::get()->scene()->rootNode();

    portal1Node = Node::create("portal1", rootNode);
    portal2Node = Node::create("portal2", rootNode);

    createVirtualCameras();
    createShaders();
}

std::shared_ptr<Portal> Portal::create(
    const std::shared_ptr<Camera>& camera
) {
    auto instance = std::make_shared<Portal>(camera);
    return instance;
}

void Portal::setPortalMesh(
    int portalIndex,
    const std::shared_ptr<MeshData> &meshData
) {
    if (portalIndex != 1 && portalIndex != 2) {
        throw std::runtime_error("Portal: incorrect portal index");
    }

    const auto portalNode = portalIndex == 1 ? portal1Node : portal2Node;

    const auto childNode = Node::create("childNode", portalNode);
    const auto mesh = MeshComponent::Factory::create(childNode, meshData, "portalMesh");

    mesh->setShader(m_basicPortalShader);
    mesh->setBeforeDrawCallback([](const std::shared_ptr<ShaderProgram>& shaderProgram) {
        shaderProgram->setUniform("color", glm::vec3(0.3, 0.3, 0.3));
    });

    const auto borderNode = Node::create("borderNode", childNode);
    const auto borderMesh = MeshComponent::Factory::create(borderNode, meshData, "portalBorderMesh");

    const auto tailNode = Node::create("tailNode", childNode);
    const auto tailMesh = MeshComponent::Factory::create(tailNode, meshData, "portalTailMesh");

    tailMesh->setShader(m_tailPortalShader);
    tailMesh->setBeforeDrawCallback([&](const auto& shader) {
        shader->setUniform("tailCameraView", m_tailVirtualCamera->viewMatrix());
        shader->setUniform("tailCameraProjection", m_tailVirtualCamera->projectionMatrix());

        shader->setTexture("albedoTexture", m_tailPortalFramebuffer->colorTextureId());
    });
}

void Portal::drawPortal(
    int portalIndex,
    const std::function<void(const std::shared_ptr<Camera>& camera)>& drawScene
) {
    if (portalIndex != 1 && portalIndex != 2) {
        throw std::runtime_error("Portal: incorrect portal index");
    }

    const auto sourcePortalNode = portalIndex == 1 ? portal1Node : portal2Node;
    const auto destPortalNode = portalIndex == 1 ? portal2Node : portal1Node;

    const auto portalMesh = sourcePortalNode->getChild("childNode")->getComponent<MeshComponent>();
    const auto portalBorderMesh =  sourcePortalNode->getChild("childNode")->getChild("borderNode")->getComponent<MeshComponent>();
    const auto portalTailMesh =  sourcePortalNode->getChild("childNode")->getChild("tailNode")->getComponent<MeshComponent>();

    auto recursiveCameras = getRecursiveCameras(sourcePortalNode, destPortalNode);

    // prepare stencil buffer
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

    for (int i=0; i < getTotalRecursionLevel(); i++) {
        glStencilFunc(GL_EQUAL, i, 0xFF);

        portalMesh->draw(recursiveCameras[i]);
    }

    // draw tail portal contents to tail FBO
    if (m_maxTailRecursionLevel > 0) {
        drawTailPortalToFramebuffer(drawScene);
    }

    // draw portal contents from deepest to shallowest
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glEnable(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    for (unsigned int i = getTotalRecursionLevel(); i >= 1; i--) {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glStencilFunc(GL_LEQUAL, i, 0xFF);

        bool isTailPortal = i > m_maxRecursionLevel;

        if (isTailPortal) {
            m_tailVirtualCamera = recursiveCameras[i - 1];
            portalTailMesh->draw(recursiveCameras[i - 1]);
        } else {
            drawScene(recursiveCameras[i]);
        }

        // draw portal border
        glStencilFunc(GL_EQUAL, i - 1, 0xFF);

        portalBorderMesh->draw(recursiveCameras[i - 1]);

        // draw portal mesh to z-buffer
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glStencilFunc(GL_LEQUAL, i, 0xFF);
        glDepthFunc(GL_ALWAYS);

        portalMesh->draw(recursiveCameras[i - 1]);

        glDepthFunc(GL_LESS);
    }
}

void Portal::drawTailPortalToFramebuffer(
    const std::function<void(const std::shared_ptr<Camera>& camera)>& drawScene
) const {
    int originalFBO = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, m_tailPortalFramebuffer->FBO());

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glDisable(GL_STENCIL_TEST);
    glStencilMask(0x00);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0, 0.5, 0, 1);

    drawScene(m_virtualCameras[m_maxRecursionLevel]);

    glBindFramebuffer(GL_FRAMEBUFFER, originalFBO);
}

void Portal::createShaders() {
    m_basicPortalShader = Engine::get()->shaderManager()->createShaderProgram(
        "shaders/solid-color/vertex.glsl",
        "shaders/solid-color/fragment.glsl",
        "basic portal shader program"
    );

    m_tailPortalShader = Engine::get()->shaderManager()->createShaderProgram(
        "shaders/tail-portal/vertex.glsl",
        "shaders/tail-portal/fragment.glsl",
        "tail portal shader program"
    );
}

void Portal::createVirtualCameras() {
    const auto virtualCamerasNode = Node::create("portalVirtualCameras", Engine::get()->scene()->rootNode());

    for (int i=0; i < getTotalRecursionLevel(); i++) {
        auto node = Node::create("virtualCameraNode"+std::to_string(i), virtualCamerasNode);
        auto virtualCamera = Camera::Factory::create(
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
    result.reserve(getTotalRecursionLevel() + 1);

    result.push_back(m_camera);

    auto pPrev = m_camera->transform()->absolutePosition();
    auto qPrev = m_camera->transform()->absoluteOrientation();

    auto [
        qDelta, pDelta
    ] = calculatePortalTransform(sourcePortal->transform(), destPortal->transform());

    for (int i = 0; i < getTotalRecursionLevel(); i++) {
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

std::pair<glm::quat, glm::vec3> Portal::calculatePortalTransform(
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

void Portal::applyCameraNearPlane() {
    auto cT = m_camera->transform();

    float dist1 = glm::dot(cT->absolutePosition() - portal1Node->transform()->absolutePosition(), portal1Node->transform()->direction());
    float dist2 = glm::dot(cT->absolutePosition() - portal2Node->transform()->absolutePosition(), portal2Node->transform()->direction());

    static bool flag = false;

    if (dist1 > 0 && dist1 < 0.15f) {
        flag = true;
        m_camera->setNearPlane(portal2Node->transform());
    }
    else if (dist2 > 0 && dist2 < 0.15f) {
        flag = true;
        m_camera->setNearPlane(portal1Node->transform());
    } else if (flag) {
        flag = false;
        m_camera->recalculateProjectionMatrix();
    }
}
}
