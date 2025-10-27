#pragma once

#include <memory>
#include <filesystem>

namespace SimpleGL {

class WindowManager;
class ShaderManager;
class MeshManager;
class TextureManager;
class Window;
class Input;
class Scene;
class Node;

class Engine {
public:
    static Engine& instance() { return m_instance; }

    std::shared_ptr<WindowManager>& windowManager() { return m_windowManager; }
    std::shared_ptr<ShaderManager>& shaderManager() { return m_shaderManager; }
    std::shared_ptr<MeshManager>& meshManager() { return m_meshManager; }
    std::shared_ptr<TextureManager>& textureManager() { return m_textureManager; }

    std::shared_ptr<Window>& mainWindow() const;
    std::shared_ptr<Scene>& scene() { return m_scene; }

    void setScene(const std::shared_ptr<Scene>& scene) { m_scene = scene; }

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

    std::filesystem::path getResourcePath(const std::filesystem::path& filePath) const;

private:
    static Engine m_instance;

    const std::filesystem::path m_resourcesDir = std::filesystem::path(__FILE__).parent_path().parent_path() / "resources";

    std::shared_ptr<WindowManager> m_windowManager;
    std::shared_ptr<ShaderManager> m_shaderManager;
    std::shared_ptr<MeshManager> m_meshManager;
    std::shared_ptr<TextureManager> m_textureManager;

    std::shared_ptr<Scene> m_scene;

    Engine();
    ~Engine();
};

}
