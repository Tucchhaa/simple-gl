#pragma once

#include <memory>
#include <vector>

#include <glad/glad.h>

namespace SimpleGL {

enum TextureType {
    Default,
    CubeMap
};

struct Texture {
    const TextureType type;

    static std::shared_ptr<Texture> create(unsigned char* data, int width, int height, GLenum format) {
        return std::make_shared<Texture>(data, width, height, format);
    }
    static std::shared_ptr<Texture> create(const std::vector<unsigned char*>& data, int width, int height, GLenum format) {
        return std::make_shared<Texture>(data, width, height, format);
    }

    Texture(unsigned char* data, int width, int height,  GLenum format): type(Default) {
        m_textureId = createTexture(data, width, height, format);
        m_samplerId = createSampler();
    }

    Texture(const std::vector<unsigned char*>& data, int width, int height,  GLenum format): type(CubeMap) {
        m_textureId = createCubeMapTexture(data, width, height, format);
        m_samplerId = createCubeMapSampler();
    }

    ~Texture() {
        glDeleteTextures(1, &m_textureId);
    }

    unsigned int textureId() const { return m_textureId; }
    unsigned int samplerId() const { return m_samplerId; }

protected:
    unsigned int m_textureId;
    unsigned int m_samplerId;

    static unsigned int createTexture(unsigned char* data, int width, int height, GLenum format);
    static unsigned int createCubeMapTexture(const std::vector<unsigned char*>& data, int width, int height, GLenum format);

    static unsigned int createSampler();
    static unsigned int createCubeMapSampler();
};

}
