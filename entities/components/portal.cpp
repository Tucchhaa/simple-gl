#include "portal.h"

#include "../../managers/engine.h"
#include "../node.h"
#include "../scene.h"
#include "camera.h"
#include "mesh.h"
#include "transform.h"

namespace SimpleGL {

std::shared_ptr<Portal> Portal::create(
    const std::shared_ptr<Node>& node,
    const std::shared_ptr<Camera>& camera,
    const std::string& name
) {
    auto instance = base_create<Portal>(node, name);

    const auto rootNode = Engine::instance().scene()->rootNode();
    instance->m_camera = camera;

    instance->portal1Node = Node::create("portal1", rootNode);
    instance->portal2Node = Node::create("portal2", rootNode);

    Node::create("meshNode", instance->portal1Node);
    Node::create("meshNode", instance->portal2Node);

    instance->createVirtualCamera(
        "portal1VirtualCamera",
        instance->m_portal1VirtualCameraNode,
        instance->portal1VirtualCamera
    );
    instance->createVirtualCamera(
        "portal2VirtualCamera",
        instance->m_portal2VirtualCameraNode,
        instance->portal2VirtualCamera
    );

    return instance;
}

void Portal::onUpdate() {
    updateVirtualCameraTransform(portal1Node, portal2Node, portal2VirtualCamera);
    updateVirtualCameraTransform(portal2Node, portal1Node, portal1VirtualCamera);
}

void Portal::setPortalMesh(const std::shared_ptr<MeshData> &meshData) const {
    if (portal1Node->getComponent<MeshComponent>() != nullptr) {
        throw std::runtime_error("Portal node already has MeshComponent");
    }

    MeshComponent::create(portal1Node->getChild("meshNode"), meshData);
    MeshComponent::create(portal2Node->getChild("meshNode"), meshData);
}

void Portal::createVirtualCamera(
    const std::string& name,
    std::shared_ptr<Node>& node,
    std::shared_ptr<Camera>& virtualCamera
) {
    node = Node::create(name);
    node->setParent(Engine::instance().scene()->rootNode());

    virtualCamera = Camera::create(
        node,
        m_camera->fov(),
        m_camera->near(),
        m_camera->far()
    );
}

void Portal::updateVirtualCameraTransform(
    const std::shared_ptr<Node>& sourcePortal,
    const std::shared_ptr<Node>& destPortal,
    const std::shared_ptr<Camera>& virtualCamera
) {
    const auto sT = sourcePortal->transform();
    const auto cT = m_camera->transform();
    const auto dT = destPortal->transform();
    const auto vT = virtualCamera->transform();

    static auto flipY = glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 1, 0));

    const glm::quat sQInverse = glm::inverse(sT->absoluteOrientation());

    const glm::quat qRel = sQInverse * cT->absoluteOrientation();
    const glm::quat qVirtual = dT->absoluteOrientation() * flipY * qRel;

    const glm::vec3 pRel = sQInverse * (cT->absolutePosition() - sT->absolutePosition());
    const glm::vec3 pVirtual = dT->absolutePosition() + dT->absoluteOrientation() * (flipY * pRel);

    vT->setPosition(pVirtual);
    vT->setOrientation(qVirtual);

    virtualCamera->applyNearPlaneClipping(destPortal->transform());
}

}
