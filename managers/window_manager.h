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

    std::shared_ptr<Window> createWindow(const std::string& title, int width, int height);

private:
    std::unordered_map<std::string, std::shared_ptr<Window>> m_windowsMap{};

    static void init();
};

}