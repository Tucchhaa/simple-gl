#include "screen_frame_buffer.h"

#include <glad/glad.h>

#include "../../entities/node.h"
#include "../../entities/shader_program.h"
#include "../../entities/window.h"
#include "../../entities/components/mesh.h"
#include "../../helpers/errors.h"

#include "../../managers/engine.h"
#include "../../managers/mesh_manager.h"

namespace SimpleGL {

std::shared_ptr<ScreenFrameBuffer> ScreenFrameBuffer::create(const std::shared_ptr<Window> &window, bool hdr) {
    auto instance = std::make_shared<ScreenFrameBuffer>(window->frameWidth(), window->frameHeight(), hdr);
    return instance;
}

ScreenFrameBuffer::ScreenFrameBuffer(int width, int height, bool hdr)
    : BaseFrameBuffer(width, height, hdr, 1)
{
    m_RBO = createDepthStencilRBO();
    m_colorTextureId = createColorTexture();

    m_quadNode = Engine::instance().meshManager()->createNodeFromMeshData("plane.obj");
    m_quadMesh = m_quadNode->getComponent<MeshComponent>();

    bindTexturesToFBO();
}

ScreenFrameBuffer::~ScreenFrameBuffer() {
    glDeleteTextures(1, &m_colorTextureId);
    glDeleteRenderbuffers(1, &m_RBO);
}

void ScreenFrameBuffer::setShader(const std::shared_ptr<ShaderProgram> &shaderProgram) const {
    m_quadMesh->setShader(shaderProgram);
    m_quadMesh->setBeforeDrawCallback([this](const std::shared_ptr<ShaderProgram>& shaderProgram) {
        shaderProgram->setTexture("frameTexture", m_colorTextureId);
    });
}

void ScreenFrameBuffer::renderFrame() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    m_quadMesh->draw();
}

void ScreenFrameBuffer::bindTexturesToFBO() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTextureId, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw incompleteFrameBuffer("screen frame buffer");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}
