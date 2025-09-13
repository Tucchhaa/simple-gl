#include "window_manager.h"

#include <sstream>

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

std::shared_ptr<Window> WindowManager::createWindow(const std::string& title, int width, int height) {
    GLFWwindow* glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    if (glfwWindow == nullptr) {
        glfwTerminate();

        std::ostringstream msg;

        msg
            << "WINDOW: "
            << "\"" << title << "\": "
            << "error occurred when creating window\n";

        throw std::runtime_error(msg.str());
    }

    glfwMakeContextCurrent(glfwWindow);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        glfwTerminate();

        std::ostringstream msg;

        msg
            << "WINDOW: "
            << "\"" << title << "\": "
            << "error occurred when loading glad\n";

        throw std::runtime_error(msg.str());
    }

    auto window = std::make_shared<Window>(glfwWindow);

    this->m_windowsMap[title] = window;

    return window;
}

void WindowManager::init() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

}
