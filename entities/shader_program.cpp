#include "shader_program.h"

#include <iostream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>

#include "../helpers/errors.h"

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

void ShaderProgram::log() const {
    std::cout << "Structure of shader program \"" + label + "\":\n";

    for (const auto& element : m_uniformsMap) {
        std::cout
            << "UNIFORM: "
            << "name: " << element.first << ", "
            << "location: " << element.second->location
            << "\n";
    }

    for (const auto& element : m_attribsMap) {
        std::cout
            << "ATTRIB: "
            << "name: " << element.first << ", "
            << "location: " << element.second->location
            << "\n";
    }
}

int ShaderProgram::getAttribLocation(const std::string &name) {
    return getAttrib(name)->location;
}

void ShaderProgram::setTexture(const std::string& name, unsigned int textureId) {
    const int textureUnit = getTextureUnitLocation(m_boundTexturesCount);

    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureId);

    this->setUniform(name, m_boundTexturesCount);

    m_boundTexturesCount += 1;
}

void ShaderProgram::setUniform(const std::string &name, float x, float y, float z, float w) {
    glUniform4f(getUniform(name)->location, x, y, z, w);
}

void ShaderProgram::setUniform(const std::string &name, int x) {
    glUniform1i(getUniform(name)->location, x);
}

void ShaderProgram::setUniform(const std::string &name, const glm::mat4 &matrix) {
    glUniformMatrix4fv(getUniform(name)->location, 1, false, glm::value_ptr(matrix));
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
        const auto resource = std::make_shared<ShaderParam>(location, size, type);

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
        const auto resource = std::make_shared<ShaderParam>(location, size, type);

        this->m_attribsMap[name] = resource;
    }
}

std::shared_ptr<ShaderParam> ShaderProgram::getUniform(const std::string &name) {
    const auto iterator = m_uniformsMap.find(name);

    if (iterator == m_uniformsMap.end()) {
        throw shaderUniformGetterFailed(label, name);
    }

    return iterator->second;
}


std::shared_ptr<ShaderParam> ShaderProgram::getAttrib(const std::string &name) {
    const auto iterator = m_attribsMap.find(name);

    if (iterator == m_attribsMap.end()) {
        log();
        throw shaderAttribGetterFailed(label, name);
    }

    return iterator->second;
}

int ShaderProgram::getTextureUnitLocation(int uniformLocation) {
    return GL_TEXTURE0 + uniformLocation;
}

}
