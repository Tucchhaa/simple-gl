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
    int ID;

    std::string label;

    ShaderProgram(int id, std::string lbl);

    int getAttribLocation(const std::string& name);

    void setUniform(const std::string& name, float x, float y, float z, float w);

private:
    std::unordered_map<std::string, ShaderParam> m_uniformsMap;

    std::unordered_map<std::string, ShaderParam> m_attribsMap;

    void processProgram();

    void processUniforms();

    void processAttribs();
};

}


