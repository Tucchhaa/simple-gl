#pragma once

#include <LinearMath/btVector3.h>

#include "../component.h"

namespace SimpleGL {

class PortalBullet : public Component {
public:
    PortalBullet(
        const std::weak_ptr<Node>& node,
        const std::string &name
    ) : Component(node, name) {}

    static std::shared_ptr<PortalBullet> create(
        const std::shared_ptr<Node>& node,
        const std::string& name = "PortalBullet"
    );

    void onStart() override;

    void onUpdate() override;

    void shoot(const std::shared_ptr<Transform>& origin, const btVector3& direction) const;

private:
    float m_bulletSpeed = 50.f;

};

}
