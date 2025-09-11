/*
TODO: This code contains memory leaks on error paths
*/

#include "shader_manager.h"

#include <fstream>
#include <sstream>
#include <iostream>

namespace SimpleGL {

ShaderManager::~ShaderManager() {
    this->dispose();
}

std::shared_ptr<ShaderProgram> ShaderManager::createShaderProgram(
    const std::filesystem::path& vertexShaderFile,
    const std::filesystem::path& fragmentShaderFile,
    const std::string& label
)
{
    const unsigned int shaderProgramID = glCreateProgram();

    if (shaderProgramID == 0) {
        std::ostringstream msg;

        msg
            << "SHADER MANAGER: "
            << "Shader program \"" << label << "\": "
            << "Failed to create shader program\n";

        throw std::runtime_error(msg.str());
    }

    const auto vertexShaderFilepath = this->m_resourcesDirPath / vertexShaderFile;
    const auto vertexShaderID = createShader(label, vertexShaderFilepath, GL_VERTEX_SHADER);

    const auto fragmentShaderFilepath = this->m_resourcesDirPath / fragmentShaderFile;
    const auto fragmentShaderID = createShader(label, fragmentShaderFilepath, GL_FRAGMENT_SHADER);

    glAttachShader(shaderProgramID, vertexShaderID);
    glAttachShader(shaderProgramID, fragmentShaderID);

    glLinkProgram(shaderProgramID);

    int success;
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &success);

    if (!success) {
        char log[512];
        glGetProgramInfoLog(shaderProgramID, 512, nullptr, log);

        std::ostringstream msg;

        msg
            << "SHADER MANAGER: "
            << "Shader program \"" << label << "\": "
            << "Shader linking error: "
            << log
            << "\n";

        throw std::runtime_error(msg.str());
    }

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    auto shaderProgram = std::make_shared<ShaderProgram>(shaderProgramID, label);

    this->m_shaderPrograms.push_back(shaderProgram);

    return shaderProgram;
}

void ShaderManager::dispose() {
    for (const auto& shaderProgram : this->m_shaderPrograms) {
        glDeleteProgram(shaderProgram->Id);
    }

    this->m_shaderPrograms.clear();
}

unsigned int ShaderManager::createShader(
    const std::string &label,
    const std::string &shaderPath,
    const GLenum shaderType
) {
    const unsigned int shaderID = glCreateShader(shaderType);

    if (shaderID == 0) {
        std::ostringstream msg;

        msg
            << "SHADER MANAGER: "
            << "Shader program \"" << label << "\": "
            << "Failed to create shader of type "
            << shaderType
            << "\n";

        throw std::runtime_error(msg.str());
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

        std::ostringstream msg;

        msg
            << "SHADER MANAGER: "
            << "Shader program \"" << label << "\": "
            << "Shader compilation error: "
            << log
            << "\n";

        throw std::runtime_error(msg.str());
    }
}

}
