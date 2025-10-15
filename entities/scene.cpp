#include "scene.h"

#include "components/light.h"

namespace SimpleGL {

Scene::Scene() {
    m_rootNode = Node::create("ROOT");
}

void Scene::addLight(const std::shared_ptr<Light> &light) {
    if (light->getType() == LightType::Direct) {
        m_directLights.push_back(std::dynamic_pointer_cast<DirectLight>(light));
    }

    else if (light->getType() == LightType::Point) {
        m_pointLights.push_back(std::dynamic_pointer_cast<PointLight>(light));
    }
}

void Scene::removeLight(const std::shared_ptr<Light> &light) {
    if (light->getType() == LightType::Direct) {
        eraseLightById(m_directLights, light->id);
    }

    else if (light->getType() == LightType::Point) {
        eraseLightById(m_pointLights, light->id);
    }
}

}
