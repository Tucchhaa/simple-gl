#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace SimpleGL {

class Window {
public:
    explicit Window(GLFWwindow* glfwWindow);

    void makeCurrent() const;

    bool isOpen() const;

    void frameRendered() const;

    float aspectRatio() const;


private:
    GLFWwindow* m_glfwWindow;
    int m_frameWidth;
    int m_frameHeight;
};

}

