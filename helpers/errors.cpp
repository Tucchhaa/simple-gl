#include "errors.h"

#include <sstream>

namespace SimpleGL {

std::runtime_error createGLFWWindowFailed(const std::string &label) {
    std::ostringstream msg;

    msg
        << "WINDOW: "
        << "\"" << label << "\": "
        << "error occurred when creating window\n";

    return std::runtime_error(msg.str());
}

std::runtime_error GLADLoadFailed(const std::string &label) {
    std::ostringstream msg;

    msg
        << "WINDOW: "
        << "\"" << label << "\": "
        << "error occurred when loading glad\n";

    return std::runtime_error(msg.str());
}

std::runtime_error createShaderProgramFailed(const std::string &label) {
    std::ostringstream msg;

    msg
        << "SHADER MANAGER: "
        << "Shader program \"" << label << "\": "
        << "Failed to create shader program\n";

    return std::runtime_error(msg.str());
}

std::runtime_error createShaderFailed(const std::string &label, GLenum shaderType) {
    std::ostringstream msg;

    msg
        << "SHADER MANAGER: "
        << "Shader program \"" << label << "\": "
        << "Failed to create shader of type "
        << shaderType
        << "\n";

    return std::runtime_error(msg.str());
}

std::runtime_error shaderCompilationFailed(const std::string &label, const char *log) {
    std::ostringstream msg;

    msg
        << "SHADER MANAGER: "
        << "Shader program \"" << label << "\": "
        << "Shader compilation error: "
        << log
        << "\n";

    return std::runtime_error(msg.str());
}

std::runtime_error shaderProgramLinkingFailed(const std::string &label, const char *log) {
    std::ostringstream msg;

    msg
        << "SHADER MANAGER: "
        << "Shader program \"" << label << "\": "
        << "Shader linking error: "
        << log
        << "\n";

    return std::runtime_error(msg.str());
}

std::runtime_error shaderUniformGetterFailed(const std::string &label, const std::string &name) {
    std::ostringstream msg;

    msg
        << "SHADER PROGRAM: "
        << "\"" << label << "\": "
        << "Uniform with name \"" << name << "\" does not exist\n";

    return std::runtime_error(msg.str());
}

std::runtime_error shaderAttribGetterFailed(const std::string &label, const std::string &name) {
    std::ostringstream msg;

    msg
        << "SHADER PROGRAM: "
        << "\"" << label << "\": "
        << "Attribute with name \"" << name << "\" does not exist\n";

    return std::runtime_error(msg.str());
}

std::runtime_error meshLoadingFailed(const std::string &resource, const std::string &error) {
    std::ostringstream msg;

    msg
        << "MESH MANAGER: "
        << "\"" << resource << "\": "
        << "Loading failed: " << error << "\n";

    return std::runtime_error(msg.str());
}

std::runtime_error meshShaderNotSet(const std::string &meshName) {
    std::ostringstream msg;

    msg
        << "MESH COMPONENT: "
        << "\"" << meshName << "\": "
        << "Render failed: ShaderProgram is nullptr\n";

    return std::runtime_error(msg.str());
}

std::runtime_error unableToLoadImage(const std::string &resource) {
    std::ostringstream msg;

    msg
        << "Texture: "
        << "\"" << resource << "\": "
        << "Unable to load the image\n";

    return std::runtime_error(msg.str());
}

std::runtime_error unsupportedImageFormat(const std::string &resource) {
    std::ostringstream msg;

    msg
        << "Texture: "
        << "\"" << resource << "\": "
        << "Unsupported image format\n";

    return std::runtime_error(msg.str());
}

std::runtime_error inconsistentTextureArrayMetadata(const std::string &name) {
    std::ostringstream msg;

    msg
        << "CubeMap Texture: "
        << "\"" << name << "\": "
        << "Images should same height, width and format\n";

    return std::runtime_error(msg.str());
}

}
