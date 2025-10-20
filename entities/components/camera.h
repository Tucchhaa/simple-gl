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

        instance->recalculateProjectionMatrix(fov, near, far);

        return instance;
    }

    const glm::mat4& viewMatrix() const { return m_viewMatrix; }

    const glm::mat4& projectionMatrix() const { return m_projectionMatrix; }

    void recalculateViewMatrix();

    void recalculateProjectionMatrix(float fov, float near, float far);

private:
    glm::mat4 m_viewMatrix = glm::mat4(1);
    glm::mat4 m_projectionMatrix = glm::mat4(1);
};

}
