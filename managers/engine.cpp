#include "engine.h"

#include "shader_manager.h"
#include "window_manager.h"
#include "mesh_manager.h"
#include "texture_manager.h"
#include "../entities/node.h"

namespace SimpleGL {

Engine Engine::m_instance;

std::shared_ptr<Window> & Engine::mainWindow() const { return m_windowManager->mainWindow(); }

std::filesystem::path Engine::getResourcePath(const std::filesystem::path& filePath) const {
    return m_resourcesDir / filePath;
}

Engine::Engine() {
    m_windowManager = WindowManager::create();
    m_shaderManager = ShaderManager::create();
    m_meshManager = MeshManager::create();
    m_textureManager = TextureManager::create();
    m_rootNode = Node::create("ROOT");
}

Engine::~Engine() {
    m_windowManager->destroyWindow(mainWindow());
}

}
