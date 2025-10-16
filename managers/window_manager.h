#pragma once

#include <memory>
#include <unordered_map>
#include <string>

namespace SimpleGL {

class Window;

class WindowManager : public std::enable_shared_from_this<WindowManager> {
public:
    static std::shared_ptr<WindowManager> create();

    ~WindowManager();

    std::shared_ptr<Window> createWindow(const std::string& label, int screenWidth, int screenHeight);

    void destroyWindow(std::shared_ptr<Window>& window);

    std::shared_ptr<Window>& mainWindow() { return m_mainWindow; }

private:
    std::shared_ptr<Window> m_mainWindow;

    std::unordered_map<std::string, std::shared_ptr<Window>> m_windowsMap{};

    WindowManager() = default;

    static void init();
};

}
