#include "shader_program.h"
#include<iostream>

namespace SimpleGL {

ShaderProgram::ShaderProgram(const int id, std::string lbl):
    ID(id),
    label(std::move(lbl)
) {
    this->processProgram();
}

int ShaderProgram::getAttribLocation(const std::string &name) {
    return this->m_attribsMap[name].location;
}

void ShaderProgram::setUniform(const std::string &name, float x, float y, float z, float w) {
    glUniform4f(this->m_uniformsMap[name].location, x, y, z, w);
}

void ShaderProgram::processProgram() {
    this->processUniforms();
    this->processAttribs();
}

void ShaderProgram::processUniforms() {
    int uniformsCount = 0;
    glGetProgramiv(this->ID, GL_ACTIVE_UNIFORMS, &uniformsCount);

    for (int i=0; i < uniformsCount; i++) {
        constexpr GLsizei nameBufferSize = 256;

        int nameLength, size;
        GLenum type;
        char nameBuffer[nameBufferSize];

        glGetActiveUniform(this->ID, i, nameBufferSize, &nameLength, &size, &type, nameBuffer);
        const int location = glGetUniformLocation(this->ID, nameBuffer);

        const std::string name(nameBuffer, nameLength);
        const ShaderParam resource = { location, size, type };

        this->m_uniformsMap[name] = resource;
    }
}

void ShaderProgram::processAttribs() {
    int attribsCount = 0;
    glGetProgramiv(this->ID, GL_ACTIVE_ATTRIBUTES, &attribsCount);

    for (int i=0; i < attribsCount; i++) {
        constexpr GLsizei nameBufferSize = 256;

        int nameLength, size;
        GLenum type;
        char nameBuffer[nameBufferSize];

        glGetActiveAttrib(this->ID, i, nameBufferSize, &nameLength, &size, &type, nameBuffer);
        const int location = glGetAttribLocation(this->ID, nameBuffer);

        const std::string name(nameBuffer, nameLength);
        const ShaderParam resource = { location, size, type };

        this->m_attribsMap[name] = resource;
    }
}


}