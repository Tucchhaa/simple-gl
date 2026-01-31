#pragma once

#include <memory>
#include <vector>

namespace SimpleGL {

enum TextureType {
    Default,
    CubeMap
};

struct Texture {
    Texture(unsigned char* data, int width, int height, int channelsNum, bool isAlbedo);

    Texture(const std::vector<unsigned char*>& data, int width, int height, int channelsNum);

    ~Texture();

    unsigned int textureId() const { return m_textureId; }
    unsigned int samplerId() const { return m_samplerId; }
    TextureType type() const { return m_type; }

protected:
    unsigned int m_textureId = 0;
    unsigned int m_samplerId = 0;

    TextureType m_type = Default;

    static unsigned int createTexture(unsigned char* data, int width, int height, int channelsNum, bool isAlbedo);
    static unsigned int createCubeMapTexture(const std::vector<unsigned char*>& data, int width, int height, int channelsNum);

    static unsigned int createSampler();
    static unsigned int createCubeMapSampler();

private:
    static int getInternalFormat(bool isAlbedo);

    static unsigned int getFormat(int channelsNum);
};

}
