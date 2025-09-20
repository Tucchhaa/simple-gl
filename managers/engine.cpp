#include "engine.h"

#include "shader_manager.h"
#include "window_manager.h"
#include "../entities/node.h"

namespace SimpleGL {

Engine Engine::m_instance;

std::shared_ptr<Window> & Engine::mainWindow() const { return m_windowManager->mainWindow(); }

Engine::Engine() {
    const std::string shadersDir = "/Users/tucha/Repositories/3D/simple-gl/resources";

    m_windowManager = WindowManager::create();
    m_shaderManager = ShaderManager::create(shadersDir);
    m_rootNode = Node::create("ROOT");
}

Engine::~Engine() {
    m_windowManager->destroyWindow(mainWindow());
}

}
