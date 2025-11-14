#pragma once

#include <memory>

#include "base_frame_buffer.h"

namespace SimpleGL {

class ShaderProgram;
class Window;
class Node;
class MeshComponent;

class ScreenFrameBuffer : public BaseFrameBuffer {
public:
    static std::shared_ptr<ScreenFrameBuffer> create(const std::shared_ptr<Window>& window, bool hdr);

    ScreenFrameBuffer(int width, int height, bool hdr);

    ~ScreenFrameBuffer();

    void setShader(const std::shared_ptr<ShaderProgram>& shaderProgram) const;

    void renderFrame() const;

private:
    unsigned int m_RBO = 0;
    unsigned int m_colorTextureId = 0;

    std::shared_ptr<Node> m_quadNode;
    std::shared_ptr<MeshComponent> m_quadMesh;

    std::shared_ptr<ShaderProgram> m_shaderProgram;

    void bindTexturesToFBO() const;
};

}
