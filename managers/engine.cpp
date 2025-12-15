#include "engine.h"

#include "shader_manager.h"
#include "window_manager.h"
#include "mesh_manager.h"
#include "physics_manager.h"
#include "texture_manager.h"
#include "metrics_manager.h"

namespace SimpleGL {

Engine Engine::m_instance;


std::shared_ptr<Window> Engine::mainWindow() const { return m_windowManager->mainWindow(); }

std::filesystem::path Engine::getResourcePath(const std::filesystem::path& filePath) const {
    return m_resourcesDir / filePath;
}

Engine::Engine() {
    m_windowManager = WindowManager::create();
    m_shaderManager = ShaderManager::create();
    m_meshManager = MeshManager::create();
    m_textureManager = TextureManager::create();
    m_physicsManager = PhysicsManager::create();
    m_metricsManager = std::make_shared<MetricsManager>();
}

Engine::~Engine() {
    m_scene.reset();
    m_physicsManager.reset();
    m_textureManager.reset();
    m_meshManager.reset();
    m_shaderManager.reset();
    m_metricsManager.reset();
    m_windowManager->destroyWindow(mainWindow());
}

}
