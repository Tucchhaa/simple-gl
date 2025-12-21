#pragma once

#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>

namespace SimpleGL {

struct SweepCallback : btCollisionWorld::ClosestConvexResultCallback {
    const btCollisionObject* ignore;

    SweepCallback(
        const btVector3& from,
        const btVector3& to,
        const btCollisionObject* ignoreObj
    ) : ClosestConvexResultCallback(from, to), ignore(ignoreObj) { }

    btScalar addSingleResult(
        btCollisionWorld::LocalConvexResult& result,
        bool normalInWorldSpace
    ) override;
};

}