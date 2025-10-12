#pragma once

#include <unordered_map>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <utility>

namespace SimpleGL
{

/// Describes uniforms and attributes
struct ShaderParam {
    int location;
    int size;
    GLenum type;

    ShaderParam(int location, int size, GLenum type)
        : location(location), size(size), type(type) {}
};

class ShaderProgram {
public:
    const unsigned int id = 0;
    const std::string label;

    static std::shared_ptr<ShaderProgram> create(unsigned int id, std::string label);

    void use();

    void log() const;

    int getAttribLocation(const std::string& name);

    void setTexture(const std::string& name, unsigned int textureId);

    void setUniform(const std::string& name, float x, float y, float z, float w);
    void setUniform(const std::string& name, glm::vec4 vector);
    void setUniform(const std::string& name, float x, float y, float z);
    void setUniform(const std::string& name, glm::vec3 vector);
    void setUniform(const std::string& name, int x);
    void setUniform(const std::string& name, float x);
    void setUniform(const std::string &name, const glm::mat4& matrix);

    bool uniformExists(const std::string& name);
    bool attribExists(const std::string& name);

private:
    std::unordered_map<std::string, std::shared_ptr<ShaderParam>> m_uniformsMap;
    std::unordered_map<std::string, std::shared_ptr<ShaderParam>> m_attribsMap;
    int m_boundTexturesCount = 0;

    ShaderProgram(unsigned int id, std::string label): id(id), label(std::move(label)) {}

    void processProgram();

    void processUniforms();

    void processAttribs();

    std::shared_ptr<ShaderParam> getUniform(const std::string& name);

    std::shared_ptr<ShaderParam> getAttrib(const std::string& name);

    static int getTextureUnitLocation(int uniformLocation) ;
};

}


