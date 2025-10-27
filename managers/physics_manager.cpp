#include "physics_manager.h"

#include "btBulletDynamicsCommon.h"

namespace SimpleGL {

std::shared_ptr<PhysicsManager> PhysicsManager::create() {
    return std::make_shared<PhysicsManager>();
}

PhysicsManager::PhysicsManager() {
    m_collisionConfiguration = std::make_shared<btDefaultCollisionConfiguration>();
    m_dispatcher = std::make_shared<btCollisionDispatcher>(m_collisionConfiguration.get());
    m_pairCache = std::make_shared<btDbvtBroadphase>();
    m_constraintSolver = std::make_shared<btSequentialImpulseConstraintSolver>();
    m_dynamicsWorld = std::make_shared<btDiscreteDynamicsWorld>(
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
