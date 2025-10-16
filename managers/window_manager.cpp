#include "window_manager.h"

#include <sstream>

#include "../entities/window.h"
#include "../helpers/errors.h"

namespace SimpleGL {

std::shared_ptr<WindowManager> WindowManager::create() {
    init();

    auto instance = std::shared_ptr<WindowManager>(new WindowManager());

    return instance;
}

WindowManager::~WindowManager() {
    for (auto& it : m_windowsMap) {
        it.second.reset();
    }

    glfwTerminate();
}

std::shared_ptr<Window> WindowManager::createWindow(const std::string& label, int screenWidth, int screenHeight) {
    GLFWwindow* glfwWindow = glfwCreateWindow(screenWidth, screenHeight, label.c_str(), nullptr, nullptr);

    if (glfwWindow == nullptr) {
        glfwTerminate();
        throw createGLFWWindowFailed(label);
    }

    glfwMakeContextCurrent(glfwWindow);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        glfwTerminate();
        throw GLADLoadFailed(label);
    }

    auto window = Window::create(label, glfwWindow);

    if (m_mainWindow == nullptr) {
        m_mainWindow = window;
    }

    this->m_windowsMap[label] = window;

    return window;
}

void WindowManager::destroyWindow(std::shared_ptr<Window>& window) {
    m_windowsMap.erase(window->label);
    window.reset();
}

void WindowManager::init() {
    static bool isInited = false;

    if (!isInited) {
        isInited = true;
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    }
}

}
