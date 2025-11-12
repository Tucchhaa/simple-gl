#pragma once

namespace SimpleGL {

class BaseFrameBuffer {
public:
    BaseFrameBuffer(int width, int height, unsigned int samples);

    ~BaseFrameBuffer();

    unsigned int FBO() const { return m_FBO; }
    int width() const { return m_width; }
    int height() const { return m_height; }

protected:
    unsigned int m_FBO = 0;
    unsigned int m_samples;
    int m_width;
    int m_height;

    static unsigned int createFBO();

    /// Creates depth-stencil render buffer object instead of texture for optimization
    unsigned int createDepthStencilRBO() const;

    unsigned int createColorTexture() const;
};

}

