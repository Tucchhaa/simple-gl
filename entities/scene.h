#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

namespace SimpleGL {

class Component;
class Node;
class Light;
class DirectLight;
class PointLight;

class Scene {
public:
    const std::shared_ptr<Node>& rootNode() const { return m_rootNode; }
    void setRootNode(const std::shared_ptr<Node> &rootNode) { m_rootNode = rootNode; }

    const std::vector<std::weak_ptr<DirectLight>>& directLights() const { return m_directLights; }
    const std::vector<std::weak_ptr<PointLight>>& pointLights() const { return m_pointLights; }

    void start();
    void update();

private:
    std::shared_ptr<Node> m_rootNode = nullptr;

    std::unordered_map<int, std::weak_ptr<Component>> m_componentsMap;
    std::vector<std::weak_ptr<DirectLight>> m_directLights;
    std::vector<std::weak_ptr<PointLight>> m_pointLights;

    void processComponents();
};

}

