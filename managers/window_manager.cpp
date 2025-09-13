#include "window_manager.h"

#include <sstream>

#include "../helpers/errors.h"

namespace SimpleGL {

WindowManager::WindowManager() {
    init();
}

WindowManager::~WindowManager() {
    for (auto& it : m_windowsMap) {
        it.second.reset();
    }

    glfwTerminate();
}

std::shared_ptr<Window> WindowManager::createWindow(const std::string& label, int screenWidth, int height) {
    GLFWwindow* glfwWindow = glfwCreateWindow(screenWidth, height, label.c_str(), nullptr, nullptr);

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

    auto* window_ptr = new Window(label, glfwWindow);
    std::shared_ptr<Window> window(window_ptr);

    this->m_windowsMap[label] = window;

    return window;
}

void WindowManager::destroyWindow(std::shared_ptr<Window>& window) {
    glfwDestroyWindow(window->m_glfwWindow);
    m_windowsMap.erase(window->label);
    window.reset();
}

void WindowManager::init() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

}
