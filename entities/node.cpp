#include "node.h"

#include <vector>
#include <queue>

#include "components/component.h"
#include "components/transform.h"

namespace SimpleGL {

std::shared_ptr<Node> Node::create(
    const std::string &name,
    const std::shared_ptr<Node>& parent
) {
    auto instance = std::make_shared<Node>(name);

    instance->m_transform = Transform::Factory::create(instance);

    if (parent) {
        instance->setParent(parent);
    }

    return instance;
}

void Node::addComponent(const std::shared_ptr<Component>& component) {
    m_components[typeid(*component)] = component;
}

void Node::setParent(const std::shared_ptr<Node> &parent) {
    auto oldParent = m_parent.lock();

    if (oldParent != nullptr) {
        std::erase(oldParent->m_children, shared_from_this());
    }

    m_parent = parent;

    parent->m_children.push_back(shared_from_this());
}

std::shared_ptr<Node> Node::getChild(const std::string& childName) const {
    for (auto child : m_children) {
        if (child->name == childName) {
            return child;
        }
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

void Node::traverseChildren(const std::function<void(const std::shared_ptr<Node>&)>& callback) {
    std::queue<std::shared_ptr<Node>> q;
    q.push(shared_from_this());

    while (!q.empty()) {
        auto currentNode = q.front();
        q.pop();

        callback(currentNode);

        for (const auto& child : currentNode->children()) {
            q.push(child);
        }
    }
}

}
