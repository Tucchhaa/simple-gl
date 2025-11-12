#include "msaa_frame_buffer.h"

#include <glad/glad.h>

#include "../../entities/window.h"
#include "../../helpers/errors.h"

namespace SimpleGL {

std::shared_ptr<MsaaFrameBuffer> MsaaFrameBuffer::create(const std::shared_ptr<Window> &window, int samples) {
    auto instance = std::make_shared<MsaaFrameBuffer>(window->frameWidth(), window->frameHeight(), samples);
    return instance;
}

MsaaFrameBuffer::MsaaFrameBuffer(int width, int height, int samples)
    : BaseFrameBuffer(width, height, samples
) {
    m_RBO = createDepthStencilRBO();
    m_colorTextureId = createColorTexture();

    bindTexturesToFBO();
}

MsaaFrameBuffer::~MsaaFrameBuffer() {
    glDeleteTextures(1, &m_colorTextureId);
    glDeleteRenderbuffers(1, &m_RBO);
}

void MsaaFrameBuffer::bindTexturesToFBO() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_colorTextureId, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw incompleteFrameBuffer("msaa frame buffer");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


}
