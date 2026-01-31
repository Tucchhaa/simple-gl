#include "window.h"

#include <sstream>

#include "input.h"

namespace SimpleGL {

Window::Window() {
    static bool isGLFWContextInited = false;

    if (!isGLFWContextInited) {
        isGLFWContextInited = true;
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    }
}

Window::~Window() {
    if (m_glfwWindow != nullptr) {
        destroy();
    }

    glfwTerminate();
}

void Window::open(int screenWidth, int screenHeight) {
    if (m_glfwWindow != nullptr) {
        throw std::runtime_error("createWindow");
    }

    m_glfwWindow = createGLFWWindow(screenWidth, screenHeight);
    m_input = std::make_unique<Input>();

    int frameWidth, frameHeight;
    float xScale, yScale;

    glfwGetFramebufferSize(m_glfwWindow, &frameWidth, &frameHeight);
    glfwGetWindowContentScale(m_glfwWindow, &xScale, &yScale);

    m_frameWidth = frameWidth;
    m_frameHeight = frameHeight;
    m_screenWidth = static_cast<int>(static_cast<float>(frameWidth) / xScale);
    m_screenHeight = static_cast<int>(static_cast<float>(frameHeight) / yScale);

    setEventCallbacks();
}

void Window::destroy() {
    glfwDestroyWindow(m_glfwWindow);
    m_glfwWindow = nullptr;
}

void Window::close() const {
    glfwSetWindowShouldClose(m_glfwWindow, GLFW_TRUE);
}

bool Window::isOpen() const {
    return !glfwWindowShouldClose(m_glfwWindow);
}

bool Window::isFocused() const {
    return glfwGetWindowAttrib(m_glfwWindow, GLFW_FOCUSED) == GLFW_TRUE;
}

void Window::getCursorPos(double* mouseX, double* mouseY) const {
    glfwGetCursorPos(m_glfwWindow, mouseX, mouseY);
}

void Window::setCursorPosToCenter() const {
    const float screenWidth = static_cast<float>(m_screenWidth);
    const float screenHeight = static_cast<float>(m_screenHeight);

    glfwSetCursorPos(m_glfwWindow, screenWidth / 2.f, screenHeight / 2.f);
}

void Window::setTitle(const std::string &title) const {
    glfwSetWindowTitle(m_glfwWindow, title.c_str());
}

void Window::pollEvents() const {
    input()->updateState();

    glfwSwapBuffers(m_glfwWindow);
    glfwPollEvents();
}

GLFWwindow* Window::createGLFWWindow(int screenWidth, int screenHeight) {
    GLFWwindow* glfwWindow = glfwCreateWindow(
        screenWidth, screenHeight, "No title", nullptr, nullptr
    );

    if (glfwWindow == nullptr) {
        glfwTerminate();
        throw std::runtime_error("glfwCreateWindow");
    }

    glfwMakeContextCurrent(glfwWindow);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        glfwTerminate();
        throw std::runtime_error("gladLoadGLLoader");
    }

    glfwSetWindowUserPointer(glfwWindow, this);

    return glfwWindow;
}

void Window::setEventCallbacks() const {
    auto resizeCallback = [](GLFWwindow* window, int frameWidth, int frameHeight) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        self->m_frameWidth = frameWidth;
        self->m_frameHeight = frameHeight;
        glViewport(0, 0, frameWidth, frameHeight);
    };

    auto keyCallback = [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        self->input()->keyCallback(key, action);
    };

    auto mouseButtonCallback = [](GLFWwindow* window, int button, int action, int mods) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        self->input()->mouseButtonCallback(button, action);
    };

    glfwSetFramebufferSizeCallback(m_glfwWindow, resizeCallback);
    glfwSetKeyCallback(m_glfwWindow, keyCallback);
    glfwSetMouseButtonCallback(m_glfwWindow, mouseButtonCallback);
}

}
