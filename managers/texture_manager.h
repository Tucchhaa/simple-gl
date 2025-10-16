#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include <glad/glad.h>

namespace SimpleGL {

struct Texture;

class TextureManager {
public:
    static std::shared_ptr<TextureManager> create() {
        return std::make_shared<TextureManager>();
    }

    std::shared_ptr<Texture> getTexture(const std::string& path, bool flip = true);

    std::shared_ptr<Texture> getCubeMapTexture(
        const std::string& name,
        const std::string& positiveXPath,
        const std::string& negativeXPath,
        const std::string& positiveYPath,
        const std::string& negativeYPath,
        const std::string& positiveZPath,
        const std::string& negativeZPath,
        bool flip
    );

private:
    std::unordered_map<std::string, std::weak_ptr<Texture>> m_textures;

    std::shared_ptr<Texture> _getTexture(
        const std::string& key,
        const std::vector<std::string>& paths,
        bool flip
    );

    static GLenum getFormat(int channelsNum);
};

}
