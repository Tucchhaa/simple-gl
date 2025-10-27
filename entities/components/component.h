#pragma once

#include <string>
#include <memory>

namespace SimpleGL {

class Node;
class Transform;

class Component : public std::enable_shared_from_this<Component> {
public:
    const int id;
    const std::string name;

    virtual ~Component() {}

    const std::shared_ptr<Transform>& transform() const;

    std::shared_ptr<Node> node() const { return m_node.lock(); }

    virtual void onStart() {}
    virtual void onUpdate() {}

protected:
    template <typename T>
    static std::shared_ptr<T> base_create(const std::shared_ptr<Node> &node, const std::string& name = "") {
        const auto instance = std::make_shared<T>(node, name);

        linkComponent(node, std::static_pointer_cast<Component>(instance));

        return instance;
    }

    Component(const std::weak_ptr<Node> &node, std::string name):
        m_node(node),
        name(std::move(name)),
        id(componentsCount++) { }

private:
    std::weak_ptr<Node> m_node;

    static int componentsCount;

    static void linkComponent(
        const std::shared_ptr<Node> &node,
        const std::shared_ptr<Component>& component
    );
};

}
