#pragma once

#include <string>

#include <glad/glad.h>

namespace SimpleGL {

std::runtime_error createGLFWWindowFailed(const std::string& label);

std::runtime_error GLADLoadFailed(const std::string& label);

std::runtime_error createShaderProgramFailed(const std::string& label);

std::runtime_error createShaderFailed(const std::string& label, GLenum shaderType);

std::runtime_error shaderCompilationFailed(const std::string& label, const char* log);

std::runtime_error shaderProgramLinkingFailed(const std::string& label, const char* log);

std::runtime_error shaderUniformGetterFailed(const std::string& label, const std::string& name);

std::runtime_error shaderAttribGetterFailed(const std::string& label, const std::string& name);

}
