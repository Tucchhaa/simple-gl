#include "texture.h"

#include <glad/glad.h>

namespace SimpleGL {

Texture::Texture(unsigned char *data, int width, int height, int channelsNum, bool isAlbedo) {
    m_textureId = createTexture(data, width, height, channelsNum, isAlbedo);
    m_samplerId = createSampler();
    m_type = Default;
}

Texture::Texture(const std::vector<unsigned char *> &data, int width, int height, int channelsNum) {
    m_textureId = createCubeMapTexture(data, width, height, channelsNum);
    m_samplerId = createCubeMapSampler();
    m_type = CubeMap;
}

Texture::~Texture() {
    glDeleteTextures(1, &m_textureId);
}

unsigned int Texture::createTexture(
    unsigned char *data, int width, int height,
    int channelsNum, bool isAlbedo
) {
    unsigned int textureId;
    int internalFormat = getInternalFormat(isAlbedo);
    unsigned int format = getFormat(channelsNum);

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(
        GL_TEXTURE_2D, 0, internalFormat,
        width, height, 0,
        format, GL_UNSIGNED_BYTE, data
    );

    glGenerateMipmap(GL_TEXTURE_2D);

    return textureId;
}

unsigned int Texture::createCubeMapTexture(
    const std::vector<unsigned char *> &data,
    int width, int height, int channelsNum
) {
    constexpr int texturesCount = 6;

    unsigned int textureId;
    int internalFormat = getInternalFormat(true);
    unsigned int format = getFormat(channelsNum);

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    for (unsigned int i = 0; i < texturesCount; i++) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, internalFormat, width, height, 0, format,
            GL_UNSIGNED_BYTE, data[i]
        );
    }

    return textureId;
}

unsigned int Texture::createSampler() {
    unsigned int samplerId;

    glGenSamplers(1, &samplerId);
    glSamplerParameteri(samplerId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(samplerId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(samplerId, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(samplerId, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return samplerId;
}

unsigned int Texture::createCubeMapSampler() {
    unsigned int samplerId;

    glGenSamplers(1, &samplerId);
    glSamplerParameteri(samplerId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(samplerId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(samplerId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(samplerId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(samplerId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return samplerId;
}

int Texture::getInternalFormat(bool isAlbedo) {
    return isAlbedo ? GL_SRGB : GL_RGB;
}

unsigned int Texture::getFormat(int channelsNum) {
    if (channelsNum == 1)
        return GL_RED;
    if (channelsNum == 3)
        return GL_RGB;
    if (channelsNum == 4)
        return GL_RGBA;

    return 0;
}

}
