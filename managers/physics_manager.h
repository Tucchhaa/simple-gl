#pragma once

#include <memory>

#include <BulletCollision/BroadphaseCollision/btBroadphaseInterface.h>
#include <BulletCollision/BroadphaseCollision/btDispatcher.h>
#include <BulletCollision/CollisionDispatch/btCollisionConfiguration.h>
#include <BulletDynamics/ConstraintSolver/btConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDynamicsWorld.h>

namespace SimpleGL {

class PhysicsManager {
public:
    PhysicsManager();
    ~PhysicsManager();

    const std::unique_ptr<btDynamicsWorld>& dynamicsWorld() { return m_dynamicsWorld; }

private:
    std::unique_ptr<btCollisionConfiguration> m_collisionConfiguration;
    std::unique_ptr<btDispatcher> m_dispatcher;
    std::unique_ptr<btConstraintSolver> m_constraintSolver;
    std::unique_ptr<btBroadphaseInterface> m_pairCache;
    std::unique_ptr<btDynamicsWorld> m_dynamicsWorld;
};

}
