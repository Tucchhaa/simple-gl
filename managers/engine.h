#pragma once

#include <memory>

namespace SimpleGL {

class WindowManager;
class ShaderManager;
class Window;
class Input;
class Node;

class Engine {
public:
    static Engine& instance() { return m_instance; }

    std::shared_ptr<WindowManager>& windowManager() { return m_windowManager; }
    std::shared_ptr<ShaderManager>& shaderManager() { return m_shaderManager; }
    std::shared_ptr<Node>& rootNode() { return m_rootNode; }

    std::shared_ptr<Window>& mainWindow() const;

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

private:
    static Engine m_instance;

    std::shared_ptr<WindowManager> m_windowManager;
    std::shared_ptr<ShaderManager> m_shaderManager;
    std::shared_ptr<Node> m_rootNode;

    Engine();
    ~Engine();
};

}
