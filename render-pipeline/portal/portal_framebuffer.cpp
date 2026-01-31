#include "portal_framebuffer.h"

#include <stdexcept>
#include <glad/glad.h>

namespace SimpleGL {

PortalFramebuffer::~PortalFramebuffer() {
    glDeleteTextures(1, &m_colorTextureId);
    glDeleteRenderbuffers(1, &m_RBO);
}

PortalFramebuffer::PortalFramebuffer(int width, int height, bool hdr)
    : BaseFrameBuffer(width, height, hdr, 1)
{
    m_RBO = createDepthStencilRBO();
    m_colorTextureId = createColorTexture();

    bindTexturesToFBO();
}

void PortalFramebuffer::bindTexturesToFBO() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTextureId, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("PORTAL FRAMEBUFFER is incomplete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}
