#include "window.h"

namespace SimpleGL {

Window::Window(GLFWwindow *glfwWindow):
    m_glfwWindow(glfwWindow)
{
    auto resizeCallback = [](GLFWwindow* window, int frameWidth, int frameHeight) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        self->m_frameWidth = frameWidth;
        self->m_frameHeight = frameHeight;
        glViewport(0, 0, frameWidth, frameHeight);
    };


    glfwSetWindowUserPointer(glfwWindow, this);
    glfwSetFramebufferSizeCallback(glfwWindow, resizeCallback);

    glfwGetFramebufferSize(glfwWindow, &m_frameWidth, &m_frameHeight);
}

void Window::makeCurrent() const {
    glfwMakeContextCurrent(m_glfwWindow);
}

bool Window::isOpen() const {
    return !glfwWindowShouldClose(m_glfwWindow);
}

void Window::frameRendered() const {
    glfwSwapBuffers(m_glfwWindow);
    glfwPollEvents();
}

float Window::aspectRatio() const {
    return static_cast<float>(m_frameWidth) / static_cast<float>(m_frameHeight);
}

}
