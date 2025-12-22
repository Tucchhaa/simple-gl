#include "teleportable.h"

#include "../mesh.h"
#include "../transform.h"
#include "../rigid_body.h"
#include "../../node.h"
#include "../../../helpers/converter.h"
#include "../../../render-pipeline/portal/portal.h"

namespace SimpleGL {

void Teleportable::onStart() {
    if (m_portal == nullptr) {
        throw std::runtime_error("Teleportable: Portal is not set");
    }

    if (m_rigidBody == nullptr) {
        throw std::runtime_error("Teleportable: Rigid Body is not set");
    }

    if (m_allowPortalGroup == -1) {
        throw std::runtime_error("Teleportable: Allow Portal Group is not set");
    }
}

void Teleportable::onUpdate() {
    if (m_isCloseEnough1) {
        teleportIfNeed(m_portal->portal1Node, m_portal->portal2Node);
    }

    if (m_isCloseEnough2) {
        teleportIfNeed(m_portal->portal2Node, m_portal->portal1Node);
    }

    toggleCollisionIfNeed();
}

void Teleportable::draw(const std::shared_ptr<Camera> &camera) const {
    if (m_isCloseEnough1) {
        drawClone(camera, m_portal->portal1Node, m_portal->portal2Node);
    }

    if (m_isCloseEnough2) {
        drawClone(camera, m_portal->portal2Node, m_portal->portal1Node);
    }
}

void Teleportable::toggleCollisionIfNeed() {
    const auto displacement1 = m_portal->portal1Node->transform()->position() - transform()->position();
    const auto displacement2 = m_portal->portal2Node->transform()->position() - transform()->position();
    const auto distance2_1 = glm::dot(displacement1, displacement1);
    const auto distance2_2 = glm::dot(displacement2, displacement2);

    m_isCloseEnough1 = distance2_1 < m_thresholdDistance2;
    m_isCloseEnough2 = distance2_2 < m_thresholdDistance2;

    if ((m_isCloseEnough1 || m_isCloseEnough2) && !m_disabledPortalCollision) {
        m_disabledPortalCollision = true;
        m_rigidBody->mask &= ~m_allowPortalGroup;
        m_rigidBody->reinit();
    }

    if (!m_isCloseEnough1 && !m_isCloseEnough2 && m_disabledPortalCollision) {
        m_disabledPortalCollision = false;
        m_rigidBody->mask |= m_allowPortalGroup;
        m_rigidBody->reinit();
    }
}

void Teleportable::teleportIfNeed(
    const std::shared_ptr<Node>& sourcePortalNode,
    const std::shared_ptr<Node>& destPortalNode
) const {
    const auto displacement = transform()->position() - sourcePortalNode->transform()->position();
    const float dot = glm::dot(displacement, sourcePortalNode->transform()->direction());

    if (dot <= 0.f) {
        glm::quat qNew, qDelta;
        glm::vec3 pNew;

        getTeleportedTransform(sourcePortalNode, destPortalNode, qNew, pNew, qDelta);

        const auto btRigidBody = m_rigidBody->getBtRigidBody();

        btTransform transform;
        transform.setOrigin(Converter::toBt(pNew));
        transform.setRotation(Converter::toBt(qNew));

        btRigidBody->setWorldTransform(transform);

        if (btRigidBody->getMotionState()) {
            btRigidBody->getMotionState()->setWorldTransform(transform);
        }

        const auto btQDelta = Converter::toBt(qDelta);
        const auto velocity = quatRotate(btQDelta, btRigidBody->getLinearVelocity());
        const auto angularVelocity = quatRotate(btQDelta, btRigidBody->getAngularVelocity());

        btRigidBody->setLinearVelocity(velocity);
        btRigidBody->setAngularVelocity(angularVelocity);
    }
}

void Teleportable::drawClone(
    const std::shared_ptr<Camera>& camera,
    const std::shared_ptr<Node>& sourcePortalNode,
    const std::shared_ptr<Node>& destPortalNode
) const {
    glm::quat qNew, qDelta;
    glm::vec3 pNew;

    getTeleportedTransform(sourcePortalNode, destPortalNode, qNew, pNew, qDelta);

    const auto& originalPosition = transform()->position();
    const auto& originalOrientation = transform()->orientation();

    transform()->setPosition(pNew);
    transform()->setOrientation(qNew);
    transform()->recalculate();

    for (const auto& mesh : m_meshes) {
        mesh->draw(camera);
    }

    transform()->setPosition(originalPosition);
    transform()->setOrientation(originalOrientation);
    transform()->recalculate();
}

void Teleportable::getTeleportedTransform(
    const std::shared_ptr<Node>& sourcePortalNode,
    const std::shared_ptr<Node> &destPortalNode,
    glm::quat& qNew,
    glm::vec3& pNew,
    glm::quat& qDelta
)  const {
    const auto [qTempDelta, pDelta] = Portal::calculatePortalTransform(
        sourcePortalNode->transform(),
        destPortalNode->transform()
    );

    qDelta = qTempDelta;

    const auto pPrev = transform()->position();
    const auto qPrev = transform()->orientation();

    pNew = pDelta + (qDelta * pPrev);
    qNew = qDelta * qPrev;
}

}
