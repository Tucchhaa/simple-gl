#pragma once

#include <memory>

namespace SimpleGL {

class ShaderProgram;
class Window;
class Node;
class MeshComponent;

struct ScreenFrameBuffer {
    static std::shared_ptr<ScreenFrameBuffer> create(const std::shared_ptr<Window>& window);

    ~ScreenFrameBuffer();

    unsigned int FBO() const { return m_FBO; }

    void setShader(const std::shared_ptr<ShaderProgram>& shaderProgram) const;

    void renderFrame() const;

private:
    unsigned int m_FBO = 0;
    unsigned int m_RBO = 0;

    unsigned int m_colorTextureId = 0;

    std::shared_ptr<Node> m_quadNode;
    std::shared_ptr<MeshComponent> m_quadMesh;

    std::shared_ptr<ShaderProgram> m_shaderProgram;

    static unsigned int createFBO();

    /// Creates depth-stencil render buffer object instead of texture for optimization
    static unsigned int createDepthStencilRBO(const std::shared_ptr<Window>& window);

    static unsigned int createColorTexture(const std::shared_ptr<Window>& window);
};

}
