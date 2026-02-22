#include "screen_frame_buffer.h"

#include <stdexcept>
#include <glad/glad.h>

namespace SimpleGL {

ScreenFrameBuffer::ScreenFrameBuffer(int width, int height, bool hdr)
    : BaseFrameBuffer(width, height, hdr, 1)
{
    m_RBO = createDepthStencilRBO();
    m_colorTextureId = createColorTexture();

    bindTexturesToFBO();
}

ScreenFrameBuffer::~ScreenFrameBuffer() {
    glDeleteTextures(1, &m_colorTextureId);
    glDeleteRenderbuffers(1, &m_RBO);
}

void ScreenFrameBuffer::bindTexturesToFBO() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTextureId, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("SCREEN FRAMEBUFFER is incomplete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}
