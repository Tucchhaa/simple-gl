#pragma once

#include <string>
#include <memory>

#include "../node.h"

namespace SimpleGL {

class Transform;

class Component : public std::enable_shared_from_this<Component> {
public:
    const int id;
    const std::string name;

    virtual ~Component() = default;

    const std::shared_ptr<Transform>& transform() const { return m_node.lock()->transform(); }

    std::shared_ptr<Node> node() const { return m_node.lock(); }

protected:
    template <typename T>
    static std::shared_ptr<T> base_create(const std::shared_ptr<Node> &node, const std::string& name = "") {
        const auto instance = std::make_shared<T>(node, name);

        node->addComponent(std::static_pointer_cast<Component>(instance));

        std::static_pointer_cast<Component>(instance)->initialize();

        return instance;
    }

    static int componentsCount;

    Component(const std::weak_ptr<Node> &node, std::string name):
        m_node(node),
        name(std::move(name)),
        id(componentsCount++) { }

    virtual void initialize() {}

private:
    std::weak_ptr<Node> m_node;
};

}