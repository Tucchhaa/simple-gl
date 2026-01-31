#include "quick_accessors.h"

#include "../managers/engine.h"
#include "../managers/physics_manager.h"
#include "../entities/window.h"

namespace SimpleGL {

const std::unique_ptr<btDynamicsWorld>& dynamicsWorld() {
    return Engine::get()->physicsManager()->dynamicsWorld();
}

const std::unique_ptr<Window>& window() {
    return Engine::get()->window();
}

const std::unique_ptr<Input>& input() {
    return Engine::get()->window()->input();
}


}