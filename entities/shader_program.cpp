#include "shader_program.h"
#include <iostream>
#include <sstream>

namespace SimpleGL {

ShaderProgram::ShaderProgram(const int id, std::string lbl):
    Id(id),
    label(std::move(lbl)
) {
    m_boundTexturesCount = 0;

    this->processProgram();
}

void ShaderProgram::use() {
    glUseProgram(this->Id);

    m_boundTexturesCount = 0;
}

int ShaderProgram::getAttribLocation(const std::string &name) {
    return this->m_attribsMap[name].location;
}

void ShaderProgram::setTexture(const std::string& name, unsigned int textureId) {
    const int textureUnit = getTextureUnitLocation(m_boundTexturesCount);

    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureId);

    this->setUniform(name, m_boundTexturesCount);

    m_boundTexturesCount += 1;
}

void ShaderProgram::setUniform(const std::string &name, float x, float y, float z, float w) {
    glUniform4f(this->m_uniformsMap[name].location, x, y, z, w);
}

void ShaderProgram::setUniform(const std::string &name, int x) {
    glUniform1i(this->m_uniformsMap[name].location, x);
}

void ShaderProgram::processProgram() {
    this->processUniforms();
    this->processAttribs();
}

void ShaderProgram::processUniforms() {
    int uniformsCount = 0;
    glGetProgramiv(this->Id, GL_ACTIVE_UNIFORMS, &uniformsCount);

    for (int i=0; i < uniformsCount; i++) {
        constexpr GLsizei nameBufferSize = 256;

        int nameLength, size;
        GLenum type;
        char nameBuffer[nameBufferSize];

        glGetActiveUniform(this->Id, i, nameBufferSize, &nameLength, &size, &type, nameBuffer);
        const int location = glGetUniformLocation(this->Id, nameBuffer);

        const std::string name(nameBuffer, nameLength);
        const ShaderParam resource = { location, size, type };

        this->m_uniformsMap[name] = resource;
    }
}

void ShaderProgram::processAttribs() {
    int attribsCount = 0;
    glGetProgramiv(this->Id, GL_ACTIVE_ATTRIBUTES, &attribsCount);

    for (int i=0; i < attribsCount; i++) {
        constexpr GLsizei nameBufferSize = 256;

        int nameLength, size;
        GLenum type;
        char nameBuffer[nameBufferSize];

        glGetActiveAttrib(this->Id, i, nameBufferSize, &nameLength, &size, &type, nameBuffer);
        const int location = glGetAttribLocation(this->Id, nameBuffer);

        const std::string name(nameBuffer, nameLength);
        const ShaderParam resource = { location, size, type };

        this->m_attribsMap[name] = resource;
    }
}

int ShaderProgram::getTextureUnitLocation(int uniformLocation) {
    return GL_TEXTURE0 + uniformLocation;
}

}
