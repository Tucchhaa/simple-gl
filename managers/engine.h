#pragma once

#include <memory>
#include <filesystem>

namespace SimpleGL {

class Window;
class ShaderManager;
class MeshManager;
class TextureManager;
class PhysicsManager;
class Input;
class Scene;
class Node;

class Engine {
public:
    static Engine& instance() { return m_instance; }

    std::shared_ptr<Window> window() { return m_window; }
    std::shared_ptr<ShaderManager> shaderManager() { return m_shaderManager; }
    std::shared_ptr<MeshManager> meshManager() { return m_meshManager; }
    std::shared_ptr<TextureManager> textureManager() { return m_textureManager; }
    std::shared_ptr<PhysicsManager> physicsManager() { return m_physicsManager; }

    std::shared_ptr<Scene> scene() { return m_scene; }

    void setScene(const std::shared_ptr<Scene>& scene) { m_scene = scene; }

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

    std::filesystem::path getResourcePath(const std::filesystem::path& filePath) const;

private:
    static Engine m_instance;

    const std::filesystem::path m_resourcesDir = std::filesystem::path(__FILE__).parent_path().parent_path() / "resources";

    std::shared_ptr<Window> m_window;
    std::shared_ptr<ShaderManager> m_shaderManager;
    std::shared_ptr<MeshManager> m_meshManager;
    std::shared_ptr<TextureManager> m_textureManager;
    std::shared_ptr<PhysicsManager> m_physicsManager;

    std::shared_ptr<Scene> m_scene;

    Engine();
    ~Engine();
};

}
