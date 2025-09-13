#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "../entities/window.h"

namespace SimpleGL {

class WindowManager {
public:
    WindowManager();

    ~WindowManager();

    std::shared_ptr<Window> createWindow(const std::string& label, int screenWidth, int height);

    void destroyWindow(std::shared_ptr<Window>& window);

private:
    std::unordered_map<std::string, std::shared_ptr<Window>> m_windowsMap{};

    static void init();
};

}
