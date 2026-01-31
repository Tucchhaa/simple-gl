#pragma once

#include "../component.h"

namespace SimpleGL {

class FreeController : public Component {
public:
    class Factory : public ComponentFactory<FreeController> {};

    const float speed = 3.0f;
    const float rotationSpeed = 2.0f;

    explicit FreeController(const std::string &name = "FreeController"): Component(name) {}

protected:
    void onUpdate() override;

private:
    bool m_canRotate = false;

    void handleInput();
};

}
