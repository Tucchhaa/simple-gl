#pragma once

#include "component.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace SimpleGL {

class Transform : public Component {
public:
    static std::shared_ptr<Transform> getGlobal();

    Transform(const std::weak_ptr<Node> &node, const std::string &name): Component(node, name) {}

    static std::shared_ptr<Transform> create(const std::shared_ptr<Node> &node, const std::string& name = "Transform") {
        return base_create<Transform>(node, name);
    }

    glm::vec3 scale() const { return m_scale; }
    glm::vec3 position() const { return m_position; }
    glm::quat orientation() const { return m_orientation; }

    void setPosition(float x, float y, float z);
    void setPosition(glm::vec3 position);

    void setOrientation(float w, float x, float y, float z);
    void setOrientation(glm::quat orientation);

    void setScale(float s);
    void setScale(float x, float y, float z);
    void setScale(glm::vec3 scale);

    /// Note:
    /// Absolute values are correct only during recalculate stage.
    /// Later parent's node may be changed, so these values are not actual anymore
    glm::vec3 absoluteScale() const { return m_absoluteScale; }
    glm::vec3 absolutePosition() const { return m_absolutePosition; }
    glm::quat absoluteOrientation() const { return m_absoluteOrientation; }

    glm::vec3 direction() const { return m_direction; }
    glm::mat4 transformMatrix() const { return m_transformMatrix; }

    void translate(const glm::vec3& vector);

    void rotate(const glm::quat& rotation, const std::shared_ptr<Transform>& transform = nullptr);

    void scaleBy(float x);

    void recalculate();

    void onUpdate() override;
private:
    bool isFirst = true;

    glm::vec3 m_scale = glm::vec3(1);
    glm::vec3 m_position = glm::vec3(0);
    glm::quat m_orientation = glm::quat(1, 0, 0, 0);

    glm::vec3 m_absoluteScale = glm::vec3(1);
    glm::vec3 m_absolutePosition = glm::vec3(0);
    glm::quat m_absoluteOrientation = glm::quat(1, 0, 0, 0);

    glm::mat4 m_transformMatrix = glm::mat4(1.0f);
    glm::vec3 m_direction = glm::vec3(0, 0, 1);

    bool m_dirty = false;
    bool m_subtreeDirty = false;
    bool m_rigidBodyDirty = false;

    void markAsDirty();
    void markAsRigidBodyDirty();

    glm::mat4 calculateTransformMatrix() const;
};

}
