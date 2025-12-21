#pragma once
#include <utility>
#include <vector>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>

namespace SimpleGL {

struct ContactResult {
    const btCollisionObject* hitObject = nullptr;
    btManifoldPoint hitPoint;

    ContactResult(const btCollisionObject* hitObject, btManifoldPoint hitPoint):
        hitObject(hitObject),
        hitPoint(std::move(hitPoint)) { }
};

struct ContactCallback : btCollisionWorld::ContactResultCallback {
    bool hasHit() const { return !results.empty(); }

    std::vector<ContactResult> results;

    btScalar addSingleResult(
        btManifoldPoint& contactPoint,
        const btCollisionObjectWrapper* colObj0Wrap,
        int, int,
        const btCollisionObjectWrapper* colObj1Wrap,
        int, int
    ) override;
};

}
