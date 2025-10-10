#pragma once

#include "component.h"

namespace SimpleGL {

class FreeController : public Component {
public:
    const float speed = 3.0f;

    const float rotationSpeed = 2.0f;

    FreeController(const std::weak_ptr<Node> &node, const std::string &name): Component(node, name) {}

    static std::shared_ptr<FreeController> create(const std::shared_ptr<Node> &node, const std::string& name = "FreeController") {
        return base_create<FreeController>(node, name);
    }

    void handleInput();

private:
    bool m_canRotate = false;
};

}
