#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

namespace SimpleGL {
class Component;
}

namespace SimpleGL {

class Node;
class Light;
class DirectLight;
class PointLight;

class Scene {
public:
    static std::shared_ptr<Scene> create() {
        return std::make_shared<Scene>();
    }

    Scene() = default;

    const std::shared_ptr<Node>& rootNode() const { return m_rootNode; }
    void setRootNode(const std::shared_ptr<Node> &rootNode) { m_rootNode = rootNode; }

    const std::vector<std::weak_ptr<DirectLight>>& directLights() const { return m_directLights; }
    const std::vector<std::weak_ptr<PointLight>>& pointLights() const { return m_pointLights; }

    void addLight(const std::shared_ptr<Light>& light);
    void removeLight(const std::shared_ptr<Light>& light);

    void addComponent(const std::shared_ptr<Component>& component);

    /// Call removeComponent when Component is deleted in runtime
    void removeComponent(const std::shared_ptr<Component>& component);

    void emitStart();
    void emitUpdate();

private:
    std::shared_ptr<Node> m_rootNode = nullptr;

    std::unordered_map<int, std::weak_ptr<Component>> m_componentsMap;
    std::vector<std::weak_ptr<DirectLight>> m_directLights;
    std::vector<std::weak_ptr<PointLight>> m_pointLights;

    template <typename ContainerType>
    void eraseLightById(ContainerType& lights, int id) {
        for (auto it = lights.begin(); it != lights.end(); ++it) {
            if (it->lock()->id == id) {
                lights.erase(it);
                break;
            }
        }
    }
};

}

