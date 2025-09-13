#pragma once

#include <unordered_map>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

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
    const int Id;

    const std::string label;

    ShaderProgram(int id, std::string lbl);

    void use();

    void log() const;

    int getAttribLocation(const std::string& name);

    void setTexture(const std::string& name, unsigned int textureId);

    void setUniform(const std::string& name, float x, float y, float z, float w);
    void setUniform(const std::string& name, int x);
    void setUniform(const std::string &name, const glm::mat4& matrix);

private:
    std::unordered_map<std::string, std::shared_ptr<ShaderParam>> m_uniformsMap;

    std::unordered_map<std::string, std::shared_ptr<ShaderParam>> m_attribsMap;

    int m_boundTexturesCount;

    void processProgram();

    void processUniforms();

    void processAttribs();

    std::shared_ptr<ShaderParam> getUniform(const std::string& name);

    std::shared_ptr<ShaderParam> getAttrib(const std::string& name);

    static int getTextureUnitLocation(int uniformLocation) ;
};

}


