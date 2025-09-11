#pragma once

#include <unordered_map>
#include <string>

#include <glad/glad.h>

namespace SimpleGL
{

/// Describes uniforms and attributes
struct ShaderParam {
    int location;
    int size;
    GLenum type;
};

class ShaderProgram {
public:
    int Id;

    std::string label;

    ShaderProgram(int id, std::string lbl);

    void use();

    int getAttribLocation(const std::string& name);

    void setTexture(const std::string& name, unsigned int textureId);

    void setUniform(const std::string& name, float x, float y, float z, float w);
    void setUniform(const std::string& name, int x);

private:
    std::unordered_map<std::string, ShaderParam> m_uniformsMap;

    std::unordered_map<std::string, ShaderParam> m_attribsMap;

    int m_boundTexturesCount;

    void processProgram();

    void processUniforms();

    void processAttribs();

    static int getTextureUnitLocation(int uniformLocation) ;
};

}


