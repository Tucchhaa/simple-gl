#pragma once

#include "../../window/framebuffers/base_frame_buffer.h"

namespace SimpleGL {

class Window;

class PortalFramebuffer : public BaseFrameBuffer {
public:
    PortalFramebuffer(int width, int height, bool hdr = true);
    ~PortalFramebuffer();

    unsigned int colorTextureId() const { return m_colorTextureId; }

private:
    unsigned int m_RBO = 0;
    unsigned int m_colorTextureId = 0;

    void bindTexturesToFBO() const;
};

}
