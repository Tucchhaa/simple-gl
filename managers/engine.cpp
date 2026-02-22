#include "engine.h"

#include "shader_manager.h"
#include "mesh_manager.h"
#include "physics_manager.h"
#include "texture_manager.h"
#include "../window/window.h"

namespace SimpleGL {

std::unique_ptr<Engine> Engine::m_instance;

std::filesystem::path Engine::getResourcePath(const std::filesystem::path& filePath) const {
    return m_resourcesDir / filePath;
}

Engine::Engine() {
    m_window = std::make_unique<Window>();
    m_shaderManager = std::make_unique<ShaderManager>();
    m_meshManager = std::make_unique<MeshManager>();
    m_textureManager = std::make_unique<TextureManager>();
    m_physicsManager = std::make_unique<PhysicsManager>();
}

Engine::~Engine() {
    m_scene.reset();
    m_physicsManager.reset();
    m_textureManager.reset();
    m_meshManager.reset();
    m_shaderManager.reset();
    m_window.reset();
    m_instance.reset();
}

}
