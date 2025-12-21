#include "scene.h"

#include <ranges>

#include "node.h"
#include "components/light.h"
#include "components/transform.h"

namespace SimpleGL {

void Scene::addLight(const std::shared_ptr<Light> &light) {
    if (light->getType() == LightType::Direct) {
        m_directLights.push_back(std::static_pointer_cast<DirectLight>(light));
    }

    else if (light->getType() == LightType::Point) {
        m_pointLights.push_back(std::static_pointer_cast<PointLight>(light));
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

void Scene::addComponent(const std::shared_ptr<Component> &component) {
    m_componentsMap[component->id] = component;
}

void Scene::removeComponent(const std::shared_ptr<Component> &component) {
    m_componentsMap.erase(component->id);
}

void Scene::emitStart() {
    for (auto component: m_componentsMap | std::views::values) {
        component.lock()->onStart();
    }
}

void Scene::emitUpdate() {
    for (auto component: m_componentsMap | std::views::values) {
        component.lock()->onUpdate();
    }
}

}
