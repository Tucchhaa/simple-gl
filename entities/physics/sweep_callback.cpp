#include "sweep_callback.h"

namespace SimpleGL {

btScalar SweepCallback::addSingleResult(
    btCollisionWorld::LocalConvexResult &result,
    bool normalInWorldSpace
) {
    if (result.m_hitCollisionObject == ignore)
        return 1.0f;

    return ClosestConvexResultCallback::addSingleResult(result, normalInWorldSpace);
}

}
