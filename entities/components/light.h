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
    explicit Light(const std::string& name): Component(name) {}

    glm::vec3 ambient = glm::vec3(0.1, 0.1, 0.1);
    glm::vec3 diffuse = glm::vec3(1, 1, 1);
    glm::vec3 specular = glm::vec3(1, 1, 1);
};

class DirectLight : public Light {
public:
    class Factory : public ComponentFactory<DirectLight> {};

    explicit DirectLight(const std::string& name = "DirectLight"): Light(name) {}
};


class PointLight : public Light {
public:
    class Factory : public ComponentFactory<PointLight> {};

    explicit PointLight(const std::string& name = "PointLight"): Light(name) {}

    /// Distance at which light from the point light is zero.
    float distance = 1.0f;
};

}
