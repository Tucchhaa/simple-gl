#include "texture_manager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "engine.h"
#include "../entities/texture.h"
#include "../helpers/errors.h"
namespace SimpleGL {

std::shared_ptr<Texture> TextureManager::getTexture(const std::string &path, bool flip) {
    return _getTexture(path, { path }, flip);
}

std::shared_ptr<Texture> TextureManager::getCubeMapTexture(
    const std::string& name,
    const std::string &positiveXPath,
    const std::string &negativeXPath,
    const std::string &positiveYPath,
    const std::string &negativeYPath,
    const std::string &positiveZPath,
    const std::string &negativeZPath,
    bool flip
) {
    return _getTexture(name, {
        positiveXPath, negativeXPath,
        positiveYPath, negativeYPath,
        positiveZPath, negativeZPath
    }, flip);
}

std::shared_ptr<Texture> TextureManager::_getTexture(
    const std::string& key,
    const std::vector<std::string>& paths,
    bool flip
) {
    if (const auto it = m_textures.find(key); it != m_textures.end()) {
        if (const auto texture = it->second.lock()) {
            return texture;
        }

        m_textures.erase(it);
    }

    stbi_set_flip_vertically_on_load(flip);

    std::vector<unsigned char*> data;
    int width = 0, height = 0;
    GLenum format = 0;
    bool isFirstTexture = true;

    for (const auto& path : paths) {
        const auto resourcePath = Engine::instance().getResourcePath(path);

        int _width, _height, channelsNum;

        unsigned char *_data = stbi_load(
            resourcePath.c_str(),
            &_width, &_height,
            &channelsNum, 0
        );

        if (_data == nullptr) {
            throw unableToLoadImage(resourcePath);
        }

        const auto _format = getFormat(channelsNum);

        if (_format == 0) {
            throw unsupportedImageFormat(resourcePath);
        }

        if (isFirstTexture) {
            width = _width;
            height = _height;
            format = _format;
        } else {
            if (width != _width || height != _height || format != _format) {
                throw inconsistentTextureArrayMetadata(key);
            }
        }

        data.push_back(_data);
        isFirstTexture = false;
    }

    std::shared_ptr<Texture> texture;
    if (data.size() == 1) {
        texture = Texture::create(data[0], width, height, format);
    }
    else if (data.size() == 6) {
        texture = Texture::create(data, width, height, format);
    }

    for (auto dataItem: data) {
        stbi_image_free(dataItem);
    }

    m_textures[key] = texture;

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
