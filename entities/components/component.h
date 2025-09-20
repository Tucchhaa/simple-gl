#pragma once

#include <string>
#include <memory>

#include "../node.h"

namespace SimpleGL {

class Transform;

class Component {
public:
    const std::string name;

    virtual ~Component() = default;

protected:
    template <typename T>
    static std::shared_ptr<T> base_create(const std::shared_ptr<Node> &node, const std::string& name = "") {
        const auto instance = std::make_shared<T>(node, name);

        node->addComponent(std::static_pointer_cast<Component>(instance));

        return instance;
    }

    Component(const std::weak_ptr<Node> &node, std::string name): m_node(node), name(std::move(name)) {}

    std::weak_ptr<Node> m_node;

    const std::shared_ptr<Transform>& transform() const { return m_node.lock()->transform(); }
};

}