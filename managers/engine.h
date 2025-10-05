#pragma once

#include <memory>
#include <filesystem>

namespace SimpleGL {

class WindowManager;
class ShaderManager;
class MeshManager;
class Window;
class Input;
class Node;

class Engine {
public:
    static Engine& instance() { return m_instance; }

    std::shared_ptr<WindowManager>& windowManager() { return m_windowManager; }
    std::shared_ptr<ShaderManager>& shaderManager() { return m_shaderManager; }
    std::shared_ptr<MeshManager>& meshManager() { return m_meshManager; }
    std::shared_ptr<Node>& rootNode() { return m_rootNode; }

    std::shared_ptr<Window>& mainWindow() const;

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

    std::filesystem::path getResourcePath(const std::filesystem::path& filePath) const;

private:
    static Engine m_instance;

    std::string m_resourcesDir = "/Users/tucha/Repositories/3D/simple-gl/resources";

    std::shared_ptr<WindowManager> m_windowManager;
    std::shared_ptr<ShaderManager> m_shaderManager;
    std::shared_ptr<MeshManager> m_meshManager;
    std::shared_ptr<Node> m_rootNode;

    Engine();
    ~Engine();
};

}
