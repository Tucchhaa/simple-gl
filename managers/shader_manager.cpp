/*
TODO: This code contains memory leaks on error paths
*/

#include "shader_manager.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "engine.h"
#include "../entities/shader_program.h"

namespace SimpleGL {

ShaderManager::~ShaderManager() {
    for (const auto& shaderProgram : this->m_shaderPrograms) {
        glDeleteProgram(shaderProgram->id);
    }

    this->m_shaderPrograms.clear();
}

std::shared_ptr<ShaderProgram> ShaderManager::createShaderProgram(
    const std::filesystem::path& vertexShaderFile,
    const std::filesystem::path& fragmentShaderFile,
    const std::string& label
)
{
    const unsigned int shaderProgramID = glCreateProgram();

    if (shaderProgramID == 0) {
        throw std::runtime_error(std::format(
            "SHADER MANAGER. glCreateProgram. Label: {}\n",
            label
        ));
    }

    const auto vertexShaderFilepath = Engine::get()->getResourcePath(vertexShaderFile);
    const auto vertexShaderID = createShader(label, vertexShaderFilepath, GL_VERTEX_SHADER);

    const auto fragmentShaderFilepath = Engine::get()->getResourcePath(fragmentShaderFile);
    const auto fragmentShaderID = createShader(label, fragmentShaderFilepath, GL_FRAGMENT_SHADER);

    glAttachShader(shaderProgramID, vertexShaderID);
    glAttachShader(shaderProgramID, fragmentShaderID);

    glLinkProgram(shaderProgramID);

    int success;
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &success);

    if (!success) {
        char log[512];
        glGetProgramInfoLog(shaderProgramID, 512, nullptr, log);

        throw std::runtime_error(std::format(
            "SHADER MANAGER. glLinkProgram. Label: {}\nLog: {}",
            label, log
        ));
    }

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    auto shaderProgram = std::make_shared<ShaderProgram>(shaderProgramID, label);

    this->m_shaderPrograms.push_back(shaderProgram);

    return shaderProgram;
}

unsigned int ShaderManager::createShader(
    const std::string &label,
    const std::string &shaderPath,
    const GLenum shaderType
) {
    const unsigned int shaderID = glCreateShader(shaderType);

    if (shaderID == 0) {
        throw std::runtime_error(std::format(
            "SHADER MANAGER. glCreateShader. Label: {}\nType: {}",
            label, shaderType
        ));
    }

    const std::string vertexShaderCode = readShaderFile(shaderPath);
    const char* shaderCodeCStr = vertexShaderCode.c_str();

    compileShader(label, shaderID, shaderCodeCStr);

    return shaderID;
}

std::string ShaderManager::readShaderFile(const std::string &filePath) {
    std::ifstream file(filePath);
    std::stringstream stream;

    stream << file.rdbuf();

    std::string code = stream.str();

    return code;
}

void ShaderManager::compileShader(
    const std::string& label,
    const unsigned int &shaderID,
    const char *code
) {
    glShaderSource(shaderID, 1, &code, nullptr);
    glCompileShader(shaderID);

    int success;

    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);

    if (!success) {
        char log[512];
        glGetShaderInfoLog(shaderID, 512, nullptr, log);

        throw std::runtime_error(std::format(
            "SHADER MANAGER. glCompileShader. Label: {}\Log: {}",
            label, log
        ));
    }
}

}
