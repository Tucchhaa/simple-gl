#pragma once
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>

namespace SimpleGL {

struct ContactCallback : btCollisionWorld::ContactResultCallback {
    bool hasHit = false;
    const btCollisionObject* hitObject = nullptr;
    btVector3 hitPointWorld;
    btVector3 hitNormalWorld;

    btScalar addSingleResult(
        btManifoldPoint& cp,
        const btCollisionObjectWrapper* colObj0Wrap,
        int, int,
        const btCollisionObjectWrapper* colObj1Wrap,
        int, int
    ) override;
};

}
