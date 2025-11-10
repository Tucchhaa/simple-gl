#pragma once

#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <memory>
#include <vector>

namespace SimpleGL {
class RigidBody;
}

namespace SimpleGL {

class Component;
class Transform;

class Node : public std::enable_shared_from_this<Node> {
public:
    friend Component;
    friend RigidBody;

    std::string name;
    bool visible = true;

    static std::shared_ptr<Node> create(const std::string& name = "Node");

    template <typename T>
    std::shared_ptr<T> getComponent() {
        const auto result = m_components.find(typeid(T));

        if (result == m_components.end()) {
            return nullptr;
        }

        return static_pointer_cast<T>(result->second);
    }

    const std::shared_ptr<Transform>& transform() const { return m_transform; }
    const std::shared_ptr<RigidBody>& rigidBody() const { return m_rigidBody; }

    std::shared_ptr<Node> parent() const { return m_parent.lock(); }
    void setParent(const std::shared_ptr<Node>& parent);

    const std::vector<std::shared_ptr<Node>>& children() const { return m_children; }
    std::shared_ptr<Node> getChild(const std::string& childName) const;

    std::vector<std::shared_ptr<Component>> components() const;

private:
    std::shared_ptr<Transform> m_transform;
    std::shared_ptr<RigidBody> m_rigidBody;

    std::unordered_map<std::type_index, std::shared_ptr<Component>> m_components;

    std::vector<std::shared_ptr<Node>> m_children;

    std::weak_ptr<Node> m_parent;

    explicit Node(std::string name): name(std::move(name)) {}

    void addComponent(const std::shared_ptr<Component>& component);
};

}
