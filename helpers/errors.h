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

std::runtime_error meshLoadingFailed(const std::string& resource, const std::string& error);

std::runtime_error meshShaderNotSet(const std::string& meshName);

std::runtime_error unableToLoadImage(const std::string& resource);

std::runtime_error unsupportedImageFormat(const std::string& resource);

std::runtime_error inconsistentTextureArrayMetadata(const std::string& name);

std::runtime_error incompleteFrameBuffer(const std::string& name);

}
