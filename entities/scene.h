#pragma once

#include <memory>
#include <vector>

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

    Scene();

    std::shared_ptr<Node> rootNode() const { return m_rootNode; }

    const std::vector<std::shared_ptr<DirectLight>>& directLights() const { return m_directLights; }
    const std::vector<std::shared_ptr<PointLight>>& pointLights() const { return m_pointLights; }

    void addLight(const std::shared_ptr<Light>& light);

    void removeLight(const std::shared_ptr<Light> &light);

private:
    std::shared_ptr<Node> m_rootNode;

    std::vector<std::shared_ptr<DirectLight>> m_directLights;
    std::vector<std::shared_ptr<PointLight>> m_pointLights;

    template <typename ContainerType>
    void eraseLightById(ContainerType& lights, int id) {
        for (auto it = lights.begin(); it != lights.end(); ++it) {
            if ((*it)->id == id) {
                lights.erase(it);
                break;
            }
        }
    }
};

}

