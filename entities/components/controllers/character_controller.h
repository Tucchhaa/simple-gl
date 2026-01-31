#pragma once

#include <glm/trigonometric.hpp>

#include "../component.h"


namespace SimpleGL {

class RigidBody;

class CharacterController : public Component {
public:
    class Factory : public ComponentFactory<CharacterController> {};

    float speed = 350.f;
    float rotationSpeed = 2.0f;

    explicit CharacterController(const std::string &name = "CharacterController"): Component(name) {}

    void setCameraNode(const std::shared_ptr<Node>& cameraNode) { m_cameraNode = cameraNode; }
    void setRigidBody(const std::shared_ptr<RigidBody>& rigidBody) { m_rigidBody = rigidBody; }

protected:
    void onStart() override;

    void onUpdate() override;

    std::shared_ptr<Node> m_cameraNode;

private:
    bool m_canRotate = false;
    float m_pitch = 0.f;
    float m_yaw = 0.f;
    const float m_maxPitch = glm::radians(89.0f);

    const float m_jumpReloadTimeMs = 100;
    float m_jumpReloadLeftMs = 0;

    std::shared_ptr<RigidBody> m_rigidBody;

    bool isTouchingGround() const;
};

}
