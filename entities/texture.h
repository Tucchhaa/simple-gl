#pragma once

#include <memory>
#include <glad/glad.h>

namespace SimpleGL {

struct Texture {
    static std::shared_ptr<Texture> create(unsigned char *data, int width, int height, GLenum format) {
        return std::make_shared<Texture>(data, width, height, format);
    }

    Texture(unsigned char *data, int width, int height,  GLenum format) {
        m_textureId = createTexture(data, width, height, format);
        m_samplerId = createSampler();
    }

    ~Texture() {
        glDeleteTextures(1, &m_textureId);
    }

    unsigned int textureId() const { return m_textureId; }
    unsigned int samplerId() const { return m_samplerId; }

private:
    unsigned int m_textureId;
    unsigned int m_samplerId;

    static unsigned int createTexture(unsigned char *data, int width, int height, GLenum format);

    static unsigned int createSampler();
};

}
