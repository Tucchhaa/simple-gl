#include "scene.h"

#include <ranges>

#include "node.h"
#include "components/light.h"
#include "components/transform.h"

namespace SimpleGL {

void Scene::start() {
    processComponents();

    for (const std::weak_ptr<Component>& component : m_componentsMap | std::views::values) {
        component.lock()->onStart();
    }
}

void Scene::update() {
    for (const std::weak_ptr<Component>& component : m_componentsMap | std::views::values) {
        component.lock()->onUpdate();
    }
}

void Scene::processComponents() {
    rootNode()->traverseChildren([this](const std::shared_ptr<Node>& currentNode) {
        for (const std::shared_ptr<Component>& component : currentNode->components()) {
            m_componentsMap[component->id] = component;

            // TODO: remove usage of dynamic cast
            if (const std::shared_ptr<PointLight>& pointLight = std::dynamic_pointer_cast<PointLight>(component)) {
                m_pointLights.push_back(pointLight);
            }

            if (const std::shared_ptr<DirectLight>& directLight = std::dynamic_pointer_cast<DirectLight>(component)) {
                m_directLights.push_back(directLight);
            }
        }
    });
}

}
