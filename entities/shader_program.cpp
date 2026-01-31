#include "shader_program.h"

#include <iostream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>
#include <utility>

#include "scene.h"
#include "components/camera.h"
#include "components/light.h"
#include "components/transform.h"
#include "../helpers/errors.h"
#include "../managers/engine.h"
#include "../entities/texture.h"

namespace SimpleGL {

unsigned int ShaderProgram::activeShaderProgramId = 0;

ShaderProgram::ShaderProgram(unsigned int id, std::string label):
    id(id),
    label(std::move(label)
) {
    processProgram();
}

void ShaderProgram::use(const std::shared_ptr<Camera> &camera) {
    if (activeShaderProgramId != id) {
        activeShaderProgramId = id;
        glUseProgram(id);

        setDirectLightsUniform();
        setPointLightsUniform();
    }

    setCameraUniforms(camera);
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
    const int result = getAttrib(name)->location;
    return result;
}

void ShaderProgram::setTexture(const std::string& name, const std::shared_ptr<Texture>& texture) {
    const GLenum target = texture->type() == Default ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;

    setTexture(name, texture->textureId(), texture->samplerId(), target);
}

void ShaderProgram::setTexture(const std::string &name, unsigned int textureId, unsigned int samplerId, GLenum target) {
    const int textureUnit = getTextureUnitLocation(m_boundTexturesCount);

    glActiveTexture(textureUnit);
    glBindTexture(target, textureId);
    glBindSampler(m_boundTexturesCount, samplerId);

    this->setUniform(name, m_boundTexturesCount);

    m_boundTexturesCount += 1;
}

void ShaderProgram::setUniform(const std::string &name, float x, float y, float z, float w) {
    glUniform4f(getUniform(name)->location, x, y, z, w);
}

void ShaderProgram::setUniform(const std::string &name, const glm::vec4 vector) {
    setUniform(name, vector.x, vector.y, vector.z, vector.w);
}

void ShaderProgram::setUniform(const std::string &name, float x, float y, float z) {
    glUniform3f(getUniform(name)->location, x, y, z);
}

void ShaderProgram::setUniform(const std::string &name, const glm::vec3 vector) {
    setUniform(name, vector.x, vector.y, vector.z);
}

void ShaderProgram::setUniform(const std::string &name, int x) {
    glUniform1i(getUniform(name)->location, x);
}

void ShaderProgram::setUniform(const std::string &name, float x) {
    glUniform1f(getUniform(name)->location, x);
}

void ShaderProgram::setUniform(const std::string &name, const glm::mat4 &matrix) {
    glUniformMatrix4fv(getUniform(name)->location, 1, false, glm::value_ptr(matrix));
}

bool ShaderProgram::uniformExists(const std::string &name) {
    const auto iterator = m_uniformsMap.find(name);
    return iterator != m_uniformsMap.end();
}

bool ShaderProgram::attribExists(const std::string &name) {
    const auto iterator = m_attribsMap.find(name);
    return iterator != m_attribsMap.end();
}

void ShaderProgram::processProgram() {
    this->processUniforms();
    this->processAttribs();
}

void ShaderProgram::processUniforms() {
    int uniformsCount = 0;
    glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &uniformsCount);

    for (int i=0; i < uniformsCount; i++) {
        constexpr GLsizei nameBufferSize = 256;

        int nameLength, size;
        GLenum type;
        char nameBuffer[nameBufferSize];

        glGetActiveUniform(id, i, nameBufferSize, &nameLength, &size, &type, nameBuffer);
        const int location = glGetUniformLocation(id, nameBuffer);

        const std::string name(nameBuffer, nameLength);
        const auto resource = std::make_shared<ShaderParam>(location, size, type);

        this->m_uniformsMap[name] = resource;
    }
}

void ShaderProgram::processAttribs() {
    int attribsCount = 0;
    glGetProgramiv(id, GL_ACTIVE_ATTRIBUTES, &attribsCount);

    for (int i=0; i < attribsCount; i++) {
        constexpr GLsizei nameBufferSize = 256;

        int nameLength, size;
        GLenum type;
        char nameBuffer[nameBufferSize];

        glGetActiveAttrib(id, i, nameBufferSize, &nameLength, &size, &type, nameBuffer);
        const int location = glGetAttribLocation(id, nameBuffer);

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

void ShaderProgram::setCameraUniforms(const std::shared_ptr<Camera> &camera) {
    if (uniformExists("view")) {
        setUniform("view", camera->viewMatrix());
    }

    if (uniformExists("projection")) {
        setUniform("projection", camera->projectionMatrix());
    }

    if (uniformExists("viewPosition")) {
        setUniform("viewPosition", camera->transform()->position());
    }
}

void ShaderProgram::setDirectLightsUniform() {
    const auto scene = Engine::get()->scene();

    if (uniformExists("directLightsNum")) {
        for (int i = 0; i < scene->directLights().size(); i++) {
            const auto light = scene->directLights()[i].lock();
            auto index_str = std::to_string(i);

            setUniform("directLights[" + index_str + "].direction", light->transform()->direction());
            setUniform("directLights[" + index_str + "].ambient", light->ambient);
            setUniform("directLights[" + index_str + "].diffuse", light->diffuse);
            setUniform("directLights[" + index_str + "].specular", light->specular);
        }

        setUniform("directLightsNum", static_cast<int>(scene->directLights().size()));
    }
}

void ShaderProgram::setPointLightsUniform() {
    const auto scene = Engine::get()->scene();

    if (uniformExists("pointLightsNum")) {
        for (int i = 0; i < scene->pointLights().size(); i++) {
            const auto light = scene->pointLights()[i].lock();
            auto index_str = std::to_string(i);

            setUniform("pointLights[" + index_str + "].position", light->transform()->position());
            setUniform("pointLights[" + index_str + "].distance", light->distance);
            setUniform("pointLights[" + index_str + "].ambient", light->ambient);
            setUniform("pointLights[" + index_str + "].diffuse", light->diffuse);
            setUniform("pointLights[" + index_str + "].specular", light->specular);
        }

        setUniform("pointLightsNum", static_cast<int>(scene->pointLights().size()));
    }
}

}
