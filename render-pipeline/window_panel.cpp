#include "window_panel.h"

#include "../managers/engine.h"
#include "../managers/shader_manager.h"
#include "../managers/mesh_manager.h"
#include "../entities/window.h"
#include "../entities/node.h"
#include "../entities/components/mesh.h"
#include "../entities/shader_program.h"
#include "../entities/components/transform.h"
#include "framebuffers/msaa_frame_buffer.h"
#include "framebuffers/screen_frame_buffer.h"

namespace SimpleGL {

WindowPanel::WindowPanel(WindowPanelLocation location, WindowPanelSettings settings):
    m_location(location), m_settings(settings)
{
    initFrameBuffers();
    initQuadNode();
}

void WindowPanel::initFrameBuffers() {
    m_msaaFrameBuffer = m_settings.msaaSamples > 1
        ? std::make_unique<MsaaFrameBuffer>(frameWidth(), frameHeight(), m_settings.hdrEnabled, m_settings.msaaSamples)
        : nullptr;

    m_screenFrameBuffer = std::make_unique<ScreenFrameBuffer>(frameWidth(), frameHeight(), m_settings.hdrEnabled);
}

void WindowPanel::initQuadNode() {
    m_quadShader = Engine::get()->shaderManager()->createShaderProgram(
        "shaders/frame/vertex.glsl",
        m_settings.hdrEnabled ? "shaders/frame/hdr-fragment.glsl" : "shaders/frame/basic-fragment.glsl",
        "frame shader"
    );

    m_quadNode = Engine::get()->meshManager()->createNodeFromMeshData("plane.obj");
    m_quadNode->transform()->setPosition(m_location.x, m_location.y, 0.f);
    m_quadNode->transform()->setScale(m_location.width, m_location.height, 1.f);
    m_quadNode->transform()->recalculateDetached();

    m_quadMesh = m_quadNode->getComponent<MeshComponent>();
    m_quadMesh->setShader(m_quadShader);
    m_quadMesh->setBeforeDrawCallback([this](const std::shared_ptr<ShaderProgram>& shaderProgram) {
        shaderProgram->setTexture("frameTexture", m_screenFrameBuffer->colorTextureId());
    });
}

int WindowPanel::frameWidth() const {
    return m_settings.frameWidth == -1
        ? Engine::get()->window()->frameWidth()
        : m_settings.frameWidth;
}

int WindowPanel::frameHeight() const {
    return m_settings.frameHeight == -1
        ? Engine::get()->window()->frameHeight()
        : m_settings.frameHeight;
}

void WindowPanel::renderToFrame(const std::function<void()> &drawCallback) const {
    const unsigned int screenFBO = m_screenFrameBuffer->FBO();
    const int frameWidth = this->frameWidth();
    const int frameHeight = this->frameHeight();

    if (m_msaaFrameBuffer != nullptr) {
        const unsigned int msaaFBO = m_msaaFrameBuffer->FBO();

        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

        drawCallback();

        // resolve ms fbo
        glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenFBO);
        glBlitFramebuffer(
            0, 0, frameWidth, frameHeight,
            0, 0, frameWidth, frameHeight,
            GL_COLOR_BUFFER_BIT, GL_NEAREST
        );
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);

        drawCallback();
    }
}

void WindowPanel::renderToScreen() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glDisable(GL_STENCIL_TEST);
    glStencilMask(0x00);

    glDisable(GL_CULL_FACE);

    m_quadMesh->draw();
}

}
