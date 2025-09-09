#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <memory>

#include <glad/glad.h>

namespace SimpleGL {

struct ShaderProgram {
    int ID;

    std::string label;

    ShaderProgram(const int id, std::string lbl):
        ID(id),
        label(std::move(lbl)) {}
};

class ShaderManager {
public:
    explicit ShaderManager(std::filesystem::path resourcesDirPath):
        m_resourcesDirPath(std::move(resourcesDirPath)) {}

    ~ShaderManager();

    std::shared_ptr<ShaderProgram> createShaderProgram(
        const std::filesystem::path& vertexShaderFile,
        const std::filesystem::path& fragmentShaderFile,
        const std::string& label
    );

    void dispose();

private:
    const std::filesystem::path m_resourcesDirPath;

    std::vector<std::shared_ptr<ShaderProgram>> m_shaderPrograms;

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
