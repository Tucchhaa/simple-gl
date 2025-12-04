#pragma once

#include <memory>

#include "../framebuffers/base_frame_buffer.h"

namespace SimpleGL {

class Window;

class PortalFramebuffer : public BaseFrameBuffer {
public:
    static std::shared_ptr<PortalFramebuffer> create(int width, int height);

    PortalFramebuffer(int width, int height, bool hdr);

    ~PortalFramebuffer();

    unsigned int colorTextureId() const { return m_colorTextureId; }

private:
    unsigned int m_RBO = 0;
    unsigned int m_colorTextureId = 0;

    void bindTexturesToFBO() const;
};

}
