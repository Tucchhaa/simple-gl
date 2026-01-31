#pragma once

#include <string>
#include <memory>
#include <utility>

class btDynamicsWorld;

namespace SimpleGL {

class Node;
class Transform;
class Window;
class Input;

class Component : public std::enable_shared_from_this<Component> {
public:
    const int id;
    const std::string name;

    explicit Component(std::string name = "Component"):
        id(componentsCount++),
        name(std::move(name)) {}

    virtual ~Component() = default;

    const std::shared_ptr<Transform>& transform() const;
    std::shared_ptr<Node> node() const { return m_node.lock(); }

    virtual void attachTo(const std::shared_ptr<Node>& node);

    virtual void onStart() {}
    virtual void onUpdate() {}

    // Quick accessors


private:
    std::weak_ptr<Node> m_node;

    static int componentsCount;
};

template<typename Derived>
class ComponentFactory {
public:
    template<typename... Args>
    static std::shared_ptr<Derived> create(const std::shared_ptr<Node>& node, Args&&... args)
    {
        auto instance = std::make_shared<Derived>(std::forward<Args>(args)...);
        instance->attachTo(node);
        return instance;
    }
};

}
