#include "window.h"

#include <string>
#include <utility>

namespace SimpleGL {

Window::Window(std::string  label, GLFWwindow *glfwWindow):
    label(std::move(label)),
    m_glfwWindow(glfwWindow)
{
    auto* input_ptr = new Input(this, glfwWindow);
    m_input = std::shared_ptr<Input>(input_ptr);

    int frameWidth, frameHeight;
    glfwGetFramebufferSize(glfwWindow, &frameWidth, &frameHeight);
    m_frameWidth = frameWidth;
    m_frameHeight = frameHeight;

    glfwSetWindowUserPointer(glfwWindow, this);
    setCallbacks();
}

void Window::setCallbacks() const {
    auto resizeCallback = [](GLFWwindow* window, int frameWidth, int frameHeight) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        self->m_frameWidth = frameWidth;
        self->m_frameHeight = frameHeight;
        glViewport(0, 0, frameWidth, frameHeight);
    };
    glfwSetFramebufferSizeCallback(m_glfwWindow, resizeCallback);
}

void Window::makeCurrent() const {
    glfwMakeContextCurrent(m_glfwWindow);
}

void Window::beforeFrameRendered() const {
    m_input->process();
}

void Window::afterFrameRendered() const {
    glfwSwapBuffers(m_glfwWindow);
    glfwPollEvents();
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
