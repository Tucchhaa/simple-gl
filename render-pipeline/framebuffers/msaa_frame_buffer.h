#pragma once

#include <memory>

#include "base_frame_buffer.h"

namespace SimpleGL {

class Window;

class MsaaFrameBuffer : public BaseFrameBuffer {
public:
    static std::shared_ptr<MsaaFrameBuffer> create(const std::shared_ptr<Window>& window, int samples);

    MsaaFrameBuffer(int width, int height, int samples);

    ~MsaaFrameBuffer();

private:
    unsigned int m_RBO = 0;
    unsigned int m_colorTextureId = 0;

    void bindTexturesToFBO() const;
};

}
