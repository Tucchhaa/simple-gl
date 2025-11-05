#include "window.h"

#include <memory>
#include <string>

#include "input.h"

namespace SimpleGL {

std::shared_ptr<Window> Window::create(const std::string &label, GLFWwindow *glfwWindow) {
    auto instance = std::shared_ptr<Window>(new Window(label, glfwWindow));

    instance->m_input = Input::create(instance);

    int frameWidth, frameHeight;
    float xScale, yScale;

    glfwGetFramebufferSize(glfwWindow, &frameWidth, &frameHeight);
    glfwGetWindowContentScale(glfwWindow, &xScale, &yScale);

    instance->m_frameWidth = frameWidth;
    instance->m_frameHeight = frameHeight;
    instance->m_screenWidth = static_cast<int>(static_cast<float>(frameWidth) / xScale);
    instance->m_screenHeight = static_cast<int>(static_cast<float>(frameHeight) / xScale);

    glfwSetWindowUserPointer(glfwWindow, instance.get());
    instance->setCallbacks();

    return instance;
}

Window::~Window() {
    glfwDestroyWindow(m_glfwWindow);
}

void Window::setCallbacks() const {
    auto resizeCallback = [](GLFWwindow* window, int frameWidth, int frameHeight) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        self->m_frameWidth = frameWidth;
        self->m_frameHeight = frameHeight;
        glViewport(0, 0, frameWidth, frameHeight);
    };
    glfwSetFramebufferSizeCallback(m_glfwWindow, resizeCallback);
    glfwSetKeyCallback(m_glfwWindow, Input::keyCallback);
}

void Window::setCursorPositionToCenter() const {
    const float screenWidth = static_cast<float>(m_screenWidth);
    const float screenHeight = static_cast<float>(m_screenHeight);

    glfwSetCursorPos(glfwWindow(), screenWidth / 2.f, screenHeight / 2.f);
}

void Window::makeCurrent() const {
    glfwMakeContextCurrent(m_glfwWindow);
}

void Window::pollEvents() const {
    glfwSwapBuffers(m_glfwWindow);
    glfwPollEvents();

    m_input->process();
}

void Window::close() const {
    glfwSetWindowShouldClose(m_glfwWindow, true);
}

void Window::setTitle(const std::string& title) const {
    glfwSetWindowTitle(m_glfwWindow, title.c_str());
}

bool Window::isOpen() const {
    return !glfwWindowShouldClose(m_glfwWindow);
}

bool Window::isFocused() const {
    return glfwGetWindowAttrib(m_glfwWindow, GLFW_FOCUSED) == GLFW_TRUE;
}

}
