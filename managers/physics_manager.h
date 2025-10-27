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
    static std::shared_ptr<PhysicsManager> create();

    PhysicsManager();

    ~PhysicsManager();

    std::shared_ptr<btDynamicsWorld> dynamicsWorld() { return m_dynamicsWorld; }

private:
    std::shared_ptr<btCollisionConfiguration> m_collisionConfiguration;
    std::shared_ptr<btDispatcher> m_dispatcher;
    std::shared_ptr<btConstraintSolver> m_constraintSolver;
    std::shared_ptr<btBroadphaseInterface> m_pairCache;
    std::shared_ptr<btDynamicsWorld> m_dynamicsWorld;
};

}
