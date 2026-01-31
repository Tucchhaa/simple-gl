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
    static void init() {
        m_instance = std::make_unique<Engine>();
    }

    static std::unique_ptr<Engine>& get() { return m_instance; }

    Engine();
    ~Engine();

    const std::unique_ptr<Window>& window() { return m_window; }
    const std::unique_ptr<ShaderManager>& shaderManager() { return m_shaderManager; }
    const std::unique_ptr<MeshManager>& meshManager() { return m_meshManager; }
    const std::unique_ptr<TextureManager>& textureManager() { return m_textureManager; }
    const std::unique_ptr<PhysicsManager>& physicsManager() { return m_physicsManager; }

    std::shared_ptr<Scene> scene() const { return m_scene.lock(); }
    void setScene(const std::shared_ptr<Scene>& scene) { m_scene = scene; }

    std::filesystem::path getResourcePath(const std::filesystem::path& filePath) const;

private:
    static std::unique_ptr<Engine> m_instance;

    const std::filesystem::path m_resourcesDir = std::filesystem::path(__FILE__).parent_path().parent_path() / "resources";

    std::unique_ptr<Window> m_window;
    std::unique_ptr<ShaderManager> m_shaderManager;
    std::unique_ptr<MeshManager> m_meshManager;
    std::unique_ptr<TextureManager> m_textureManager;
    std::unique_ptr<PhysicsManager> m_physicsManager;

    std::weak_ptr<Scene> m_scene;
};

}
