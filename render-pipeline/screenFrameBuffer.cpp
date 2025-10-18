#include "screenFrameBuffer.h"

#include <glad/glad.h>

#include "../entities/shader_program.h"
#include "../entities/window.h"
#include "../entities/components/mesh.h"
#include "../helpers/errors.h"

#include "../managers/engine.h"
#include "../managers/mesh_manager.h"

namespace SimpleGL {

std::shared_ptr<ScreenFrameBuffer> ScreenFrameBuffer::create(const std::shared_ptr<Window> &window) {
    auto instance = std::make_shared<ScreenFrameBuffer>();

    instance->m_FBO = createFBO();
    instance->m_RBO = createDepthStencilRBO(window);

    instance->m_colorTextureId = createColorTexture(window);

    instance->m_quadNode = Engine::instance().meshManager()->getMesh("plane.obj");
    instance->m_quadMesh = instance->m_quadNode->children().at(0)->getComponent<MeshComponent>();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, instance->m_colorTextureId, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, instance->m_RBO);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw incompleteFrameBuffer("screen frame buffer");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return instance;
}

ScreenFrameBuffer::~ScreenFrameBuffer() {
    glDeleteTextures(1, &m_colorTextureId);
    glDeleteRenderbuffers(1, &m_RBO);
    glDeleteFramebuffers(1, &m_FBO);
}

void ScreenFrameBuffer::setShader(const std::shared_ptr<ShaderProgram> &shaderProgram) const {
    m_quadMesh->setShader(shaderProgram);
    m_quadMesh->setBeforeDrawCallback([this](const std::shared_ptr<ShaderProgram>& shaderProgram) {
        shaderProgram->setTexture("frameTexture", m_colorTextureId);
    });
}

void ScreenFrameBuffer::renderFrame() const {
    m_quadMesh->draw();
}

unsigned int ScreenFrameBuffer::createFBO() {
    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    return FBO;
}

unsigned int ScreenFrameBuffer::createDepthStencilRBO(const std::shared_ptr<Window> &window) {
    unsigned int RBO;
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window->frameWidth(), window->frameHeight());

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    return RBO;
}

unsigned int ScreenFrameBuffer::createColorTexture(const std::shared_ptr<Window> &window) {
    unsigned int textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB,
        window->frameWidth(), window->frameHeight(),
        0, GL_RGB, GL_UNSIGNED_BYTE, nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureId;
}

}
