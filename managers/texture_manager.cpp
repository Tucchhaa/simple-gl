#include "texture_manager.h"

#include <format>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "engine.h"
#include "../entities/texture.h"

namespace SimpleGL {

std::shared_ptr<Texture> TextureManager::getTexture(const std::string &path, bool isAlbedo, bool flip) {
    return _getTexture(path, { path }, isAlbedo, flip);
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
    }, true, flip);
}

std::shared_ptr<Texture> TextureManager::_getTexture(
    const std::string& key,
    const std::vector<std::string>& paths,
    bool isAlbedo,
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
    int width = 0, height = 0, channelsNum = 0;
    bool isFirstTexture = true;

    for (const auto& path : paths) {
        const auto resourcePath = Engine::get()->getResourcePath(path);

        int itemWidth, itemHeight, itemChannelsNum;

        unsigned char *_data = stbi_load(
            resourcePath.c_str(),
            &itemWidth, &itemHeight,
            &itemChannelsNum, 0
        );

        if (_data == nullptr) {
            throw std::runtime_error(std::format(
                "TEXTURE MANAGER. stbi_load. Resource: {}",
                resourcePath.c_str()
            ));
        }

        if (itemChannelsNum != 1 && itemChannelsNum != 3 && itemChannelsNum != 4) {
            throw std::runtime_error(std::format(
                "TEXTURE MANAGER. Unsupported image format. Resource: {}",
                resourcePath.c_str()
            ));
        }

        if (isFirstTexture) {
            width = itemWidth;
            height = itemHeight;
            channelsNum = itemChannelsNum;
        } else {
            if (width != itemWidth || height != itemHeight || channelsNum != itemChannelsNum) {
                throw std::runtime_error(std::format(
                "TEXTURE MANAGER. Incosistent texture array metadata. Resource: {}",
                    resourcePath.c_str()
                ));
            }
        }

        data.push_back(_data);
        isFirstTexture = false;
    }

    std::shared_ptr<Texture> texture;
    if (data.size() == 1) {
        texture = std::make_shared<Texture>(data[0], width, height, channelsNum, isAlbedo);
    }
    else if (data.size() == 6) {
        texture = std::make_shared<Texture>(data, width, height, channelsNum);
    }

    for (auto dataItem: data) {
        stbi_image_free(dataItem);
    }

    m_textures[key] = texture;

    return texture;
}

}
