#include "contact_callback.h"

namespace SimpleGL {

btScalar ContactCallback::addSingleResult(btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0Wrap, int, int,
    const btCollisionObjectWrapper *colObj1Wrap, int, int) {
    hasHit = true;
    hitObject = colObj1Wrap->getCollisionObject();
    hitPointWorld = cp.m_positionWorldOnB;
    hitNormalWorld = cp.m_normalWorldOnB;
    return 0;
}

}
