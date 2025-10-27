#include "light.h"

#include "../scene.h"
#include "../../managers/engine.h"

namespace SimpleGL {

Light::Light(const std::weak_ptr<Node> &node, const std::string &name): Component(node, name) { }

std::shared_ptr<DirectLight> DirectLight::create(const std::shared_ptr<Node> &node, const std::string &name) {
    auto instance = base_create<DirectLight>(node, name);
    Engine::instance().scene()->addLight(instance);
    return instance;
}

std::shared_ptr<PointLight> PointLight::create(const std::shared_ptr<Node> &node, const std::string &name) {
    auto instance = base_create<PointLight>(node, name);
    Engine::instance().scene()->addLight(instance);
    return instance;
}

}
