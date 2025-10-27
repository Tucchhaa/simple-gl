#include "node.h"

#include "components/component.h"
#include "components/transform.h"
#include <vector>

namespace SimpleGL {

std::shared_ptr<Node> Node::create(const std::string &name) {
    auto instance = std::shared_ptr<Node>(new Node(name));

    auto transform = Transform::create(instance);
    instance->m_transform = transform;
    instance->addComponent(transform);

    return instance;
}

void Node::addComponent(const std::shared_ptr<Component>& component) {
    m_components[typeid(*component)] = component;
}

void Node::setParent(const std::shared_ptr<Node> &parent) {
    auto oldParent = m_parent.lock();
    auto my = shared_from_this();

    if (oldParent != nullptr) {
        std::erase(oldParent->m_children, shared_from_this());
    }

    m_parent = parent;

    parent->m_children.push_back(shared_from_this());
}

std::shared_ptr<Node> Node::getChild(const std::string& childName) const {
    for (auto child : m_children) {
        if (child->name == childName) {}
        return child;
    }

    return nullptr;
}

std::vector<std::shared_ptr<Component>> Node::components() const {
    std::vector<std::shared_ptr<Component>> result;
    result.reserve(m_components.size());

    for (const auto& entry : m_components) {
        result.push_back(entry.second);
    }

    return result;
}

}
