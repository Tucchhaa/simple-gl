#include "contact_callback.h"

namespace SimpleGL {

btScalar ContactCallback::addSingleResult(
    btManifoldPoint &contactPoint,
    const btCollisionObjectWrapper *colObj0Wrap,
    int, int,
    const btCollisionObjectWrapper *colObj1Wrap,
    int, int
) {
    // hasHit = true;
    results.emplace_back(colObj1Wrap->getCollisionObject(), contactPoint);
    // hitObject = colObj1Wrap->getCollisionObject();
    // hitPoint = contactPoint;
    return 0;
}

}
