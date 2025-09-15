#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "../entities/window.h"

namespace SimpleGL {

class WindowManager : std::enable_shared_from_this<WindowManager> {
public:
    static std::shared_ptr<WindowManager> create();

    ~WindowManager();

    std::shared_ptr<Window> createWindow(const std::string& label, int screenWidth, int height);

    void destroyWindow(std::shared_ptr<Window>& window);

private:
    std::unordered_map<std::string, std::shared_ptr<Window>> m_windowsMap{};

    WindowManager() = default;

    static void init();
};

}
