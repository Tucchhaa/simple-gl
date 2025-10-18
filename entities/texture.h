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
    static std::shared_ptr<Texture> create(unsigned char* data, int width, int height, GLenum format) {
        auto instance = std::make_shared<Texture>();

        instance->m_textureId = createTexture(data, width, height, format);
        instance->m_samplerId = createSampler();
        instance->m_type = Default;

        return instance;
    }
    static std::shared_ptr<Texture> create(const std::vector<unsigned char*>& data, int width, int height, GLenum format) {
        auto instance = std::make_shared<Texture>();

        instance->m_textureId = createCubeMapTexture(data, width, height, format);
        instance->m_samplerId = createCubeMapSampler();
        instance->m_type = CubeMap;

        return instance;
    }

    ~Texture() {
        glDeleteTextures(1, &m_textureId);
    }

    unsigned int textureId() const { return m_textureId; }
    unsigned int samplerId() const { return m_samplerId; }
    TextureType type() const { return m_type; }

protected:
    unsigned int m_textureId = 0;
    unsigned int m_samplerId = 0;

    TextureType m_type = Default;

    static unsigned int createTexture(unsigned char* data, int width, int height, GLenum format);
    static unsigned int createCubeMapTexture(const std::vector<unsigned char*>& data, int width, int height, GLenum format);

    static unsigned int createSampler();
    static unsigned int createCubeMapSampler();
};

}
