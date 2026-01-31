#include "physics_manager.h"

#include "btBulletDynamicsCommon.h"

namespace SimpleGL {

PhysicsManager::PhysicsManager() {
    m_collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
    m_dispatcher = std::make_unique<btCollisionDispatcher>(m_collisionConfiguration.get());
    m_pairCache = std::make_unique<btDbvtBroadphase>();
    m_constraintSolver = std::make_unique<btSequentialImpulseConstraintSolver>();
    m_dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(
        m_dispatcher.get(),
        m_pairCache.get(),
        m_constraintSolver.get(),
        m_collisionConfiguration.get()
    );
}

PhysicsManager::~PhysicsManager() {
    m_dynamicsWorld.reset();
    m_constraintSolver.reset();
    m_pairCache.reset();
    m_dispatcher.reset();
    m_collisionConfiguration.reset();
}

}
