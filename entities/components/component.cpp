#include "component.h"

#include "../node.h"
#include "../scene.h"
#include "../../managers/engine.h"

namespace SimpleGL {

int Component::componentsCount = 0;

const std::shared_ptr<Transform> & Component::transform() const { return m_node.lock()->transform(); }

void Component::linkComponent(
    const std::shared_ptr<Node> &node,
    const std::shared_ptr<Component>& component
) {
    node->addComponent(component);
    Engine::instance().scene()->addComponent(component);
}

}
