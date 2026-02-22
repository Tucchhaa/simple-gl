#include "base_frame_buffer.h"

#include <glad/glad.h>

namespace SimpleGL {

BaseFrameBuffer::BaseFrameBuffer(int width, int height, bool hdr, unsigned int samples)
    : m_width(width), m_height(height), m_hdr(hdr), m_samples(samples)
{
    m_FBO = createFBO();
}

BaseFrameBuffer::~BaseFrameBuffer() {
    glDeleteFramebuffers(1, &m_FBO);
}

unsigned int BaseFrameBuffer::createFBO() {
    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return FBO;
}

unsigned int BaseFrameBuffer::createDepthStencilRBO() const {
    unsigned int RBO;
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);

    if (m_samples > 1) {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, m_width, m_height);
    } else {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    }

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    return RBO;
}

unsigned int BaseFrameBuffer::createColorTexture() const {
    unsigned int textureId;
    glGenTextures(1, &textureId);

    const int internalFormat = getColorTextureInternalFormat();

    if (m_samples > 1) {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureId);
        glTexImage2DMultisample(
            GL_TEXTURE_2D_MULTISAMPLE, 4, internalFormat,
            m_width, m_height,
            GL_TRUE
        );
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    } else {
        int type = getColorTextureType();

        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(
            GL_TEXTURE_2D, 0, internalFormat,
            m_width, m_height,
            0, GL_RGBA, type, nullptr
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    return textureId;
}

int BaseFrameBuffer::getColorTextureInternalFormat() const {
    return m_hdr
        ? GL_RGBA16F
        : GL_RGBA;
}

int BaseFrameBuffer::getColorTextureType() const {
    return m_hdr
        ? GL_FLOAT
        : GL_UNSIGNED_BYTE;
}

}
