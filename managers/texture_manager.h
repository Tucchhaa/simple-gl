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

    std::shared_ptr<Texture> getTexture(const std::string& path);

private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;

    static GLenum getFormat(int channelsNum);
};

}
