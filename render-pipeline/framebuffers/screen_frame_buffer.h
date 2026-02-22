#pragma once

#include "base_frame_buffer.h"

namespace SimpleGL {

class ScreenFrameBuffer : public BaseFrameBuffer {
public:
    ScreenFrameBuffer(int width, int height, bool hdr);
    ~ScreenFrameBuffer();

    unsigned int colorTextureId() const { return m_colorTextureId; }

private:
    unsigned int m_RBO = 0;
    unsigned int m_colorTextureId = 0;

    void bindTexturesToFBO() const;
};

}
