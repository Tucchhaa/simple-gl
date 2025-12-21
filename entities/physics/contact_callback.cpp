#include "contact_callback.h"

namespace SimpleGL {

btScalar ContactCallback::addSingleResult(
    btManifoldPoint &contactPoint,
    const btCollisionObjectWrapper *colObj0Wrap,
    int, int,
    const btCollisionObjectWrapper *colObj1Wrap,
    int, int
) {
    const btCollisionObject* a = colObj0Wrap->getCollisionObject();
    const btCollisionObject* b = colObj1Wrap->getCollisionObject();

    const btBroadphaseProxy* pa = a->getBroadphaseHandle();
    const btBroadphaseProxy* pb = b->getBroadphaseHandle();
    if (!pa || !pb) return 0;

    const int aGroup = pa->m_collisionFilterGroup;
    const int aMask  = pa->m_collisionFilterMask;
    const int bGroup = pb->m_collisionFilterGroup;
    const int bMask  = pb->m_collisionFilterMask;

    const bool shouldCollide =
        (aGroup & bMask) != 0 &&
        (bGroup & aMask) != 0;

    if (!shouldCollide) return 0;

    results.emplace_back(b, contactPoint);

    return 0;
}

}
