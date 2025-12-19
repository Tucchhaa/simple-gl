#pragma once

#include <glm/trigonometric.hpp>

#include "../component.h"


namespace SimpleGL {

class RigidBody;

class CharacterController : public Component {
public:
    float speed = 350.f;
    float rotationSpeed = 2.0f;

    CharacterController(
        const std::weak_ptr<Node>& node,
        const std::string &name
    ): Component(node, name) {}

    static std::shared_ptr<CharacterController> create(
        const std::shared_ptr<Node>& node,
        const std::string& name = "CharacterController"
    ) {
        auto instance = base_create<CharacterController>(node, name);
        return instance;
    }

    void setCameraNode(const std::shared_ptr<Node>& cameraNode) { m_cameraNode = cameraNode; }
    void setRigidBody(const std::shared_ptr<RigidBody>& rigidBody) { m_rigidBody = rigidBody; }

protected:
    void onStart() override;

    void onUpdate() override;

private:
    bool m_canRotate = false;
    float m_pitch = 0.f;
    float m_yaw = 0.f;
    const float m_maxPitch = glm::radians(89.0f);

    std::shared_ptr<Node> m_cameraNode;
    std::shared_ptr<RigidBody> m_rigidBody;

    bool canJump() const;
};

}
