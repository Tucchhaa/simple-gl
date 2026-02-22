#include "component.h"

#include "../node.h"

namespace SimpleGL {

int Component::componentsCount = 0;

const std::shared_ptr<Transform> & Component::transform() const { return m_node.lock()->transform(); }

void Component::attachTo(const std::shared_ptr<Node> &node) {
    m_node = node;
    node->addComponent(shared_from_this());
}

}
