#pragma once

#include <glm/vec3.hpp>

#include "component.h"

namespace SimpleGL {

enum LightType {
    Direct,
    Point,
};

class Light : public Component {
public:
    Light(const std::weak_ptr<Node> &node, const std::string &name);

    virtual LightType getType() = 0;

    glm::vec3 ambient = glm::vec3(0.1, 0.1, 0.1);
    glm::vec3 diffuse = glm::vec3(1, 1, 1);
    glm::vec3 specular = glm::vec3(1, 1, 1);
};

class DirectLight : public Light {
public:
    LightType getType() override { return Direct; }

    DirectLight(const std::weak_ptr<Node> &node, const std::string &name): Light(node, name) {}

    static std::shared_ptr<DirectLight> create(
        const std::shared_ptr<Node> &node,
        const std::string& name = "DirectLight"
    );
};

class PointLight : public Light {
public:
    LightType getType() override { return Point; }

    PointLight(const std::weak_ptr<Node> &node, const std::string &name): Light(node, name) {}

    static std::shared_ptr<PointLight> create(
        const std::shared_ptr<Node> &node,
        const std::string& name = "PointLight"
    );

    /// Distance at which light from the point light is zero.
    float distance = 1.0f;
};

}
