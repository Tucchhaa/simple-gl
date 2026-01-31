#pragma once

#include "component.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace SimpleGL {

class Camera : public Component {
public:
    class Factory : public ComponentFactory<Camera> {};

    Camera(float fov, float near, float far, const std::string& name = "Camera"):
        Component(name), m_fov(fov), m_near(near), m_far(far)
    {
        recalculateProjectionMatrix();
    }

    float fov() const { return m_fov; }
    float near() const { return m_near; }
    float far() const { return m_far; }

    const glm::mat4& viewMatrix() const { return m_viewMatrix; }
    const glm::mat4& projectionMatrix() const { return m_projectionMatrix; }

    void recalculateViewMatrix();
    void recalculateProjectionMatrix();

    void setNearPlane(const std::shared_ptr<Transform>& planeTransform);

    glm::mat4 calculateViewNormalMatrix() const;

private:
    float m_fov = 0;
    float m_near = 0;
    float m_far = 0;

    glm::mat4 m_viewMatrix = glm::mat4(1);
    glm::mat4 m_projectionMatrix = glm::mat4(1);
};

}
