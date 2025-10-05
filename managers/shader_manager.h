#pragma once

#include <string>
#include <utility>
#include <vector>
#include <filesystem>
#include <memory>

#include "../entities/shader_program.h"

namespace SimpleGL {

class ShaderManager {
public:
    static std::shared_ptr<ShaderManager> create();

    ~ShaderManager();

    std::shared_ptr<ShaderProgram> createShaderProgram(
        const std::filesystem::path& vertexShaderFile,
        const std::filesystem::path& fragmentShaderFile,
        const std::string& label
    );

private:
    std::vector<std::shared_ptr<ShaderProgram>> m_shaderPrograms;

    ShaderManager() = default;

    static unsigned int createShader(
        const std::string& label,
        const std::string& shaderPath,
        GLenum shaderType
    );

    static std::string readShaderFile(const std::string& filePath);

    static void compileShader(
        const std::string& label,
        const unsigned int& shaderID,
        const char* code
    );
};

}
