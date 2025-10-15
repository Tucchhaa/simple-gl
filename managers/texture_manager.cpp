#include "texture_manager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "engine.h"
#include "../entities/texture.h"
#include "../helpers/errors.h"

namespace SimpleGL {

std::shared_ptr<Texture> TextureManager::getTexture(const std::string &path) {
    const auto resourcePath = Engine::instance().getResourcePath(path);

    if (const auto cachedTexture = m_textures.find(resourcePath); cachedTexture != m_textures.end()) {
        return cachedTexture->second;
    }

    stbi_set_flip_vertically_on_load(true);

    int width, height, channelsNum;
    unsigned char *data = stbi_load(
        resourcePath.c_str(),
        &width, &height,
        &channelsNum, 0
    );

    if (data == nullptr) {
        throw unableToLoadImage(resourcePath);
    }

    const auto format = getFormat(channelsNum);

    if (format == 0) {
        throw unsupportedImageFormat(resourcePath);
    }

    auto texture = Texture::create(data, width, height, format);

    m_textures[path] = texture;
    stbi_image_free(data);

    return texture;
}

GLenum TextureManager::getFormat(int channelsNum) {
    if (channelsNum == 1)
        return GL_RED;
    if (channelsNum == 3)
        return GL_RGB;
    if (channelsNum == 4)
        return GL_RGBA;

    return 0;
}

}
