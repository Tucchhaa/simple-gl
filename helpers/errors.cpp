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

}
