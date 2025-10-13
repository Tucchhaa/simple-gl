#include "texture.h"

namespace SimpleGL {

unsigned int Texture::createTexture(unsigned char *data, int width, int height, GLenum format) {
    unsigned int textureId;

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(
        GL_TEXTURE_2D, 0, format,
        width, height, 0,
        format, GL_UNSIGNED_BYTE, data
    );
    glGenerateMipmap(GL_TEXTURE_2D);

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

}
