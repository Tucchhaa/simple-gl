#pragma once

#include "component.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace SimpleGL {

class Camera : public Component {
public:
    Camera(const std::weak_ptr<Node> &node, const std::string &name): Component(node, name) {}

    static std::shared_ptr<Camera> create(
        const std::shared_ptr<Node> &node,
        float fov, float near, float far,
        const std::string& name = "Camera"
    ) {
        auto instance = base_create<Camera>(node, name);

        instance->m_fov = fov;
        instance->m_near =near;
        instance->m_far = far;
        instance->recalculateProjectionMatrix();

        return instance;
    }

    float fov() const { return m_fov; }
    float near() const { return m_near; }
    float far() const { return m_far; }

    const glm::mat4& viewMatrix() const { return m_viewMatrix; }
    const glm::mat4& projectionMatrix() const { return m_projectionMatrix; }

    void onUpdate() override;

    void recalculateViewMatrix();
    void recalculateProjectionMatrix();

    void applyNearPlaneClipping(const std::shared_ptr<Transform>& planeTransform);

    glm::mat4 calculateViewNormalMatrix() const;

private:
    float m_fov = 0;
    float m_near = 0;
    float m_far = 0;

    glm::mat4 m_viewMatrix = glm::mat4(1);
    glm::mat4 m_projectionMatrix = glm::mat4(1);
};

}
