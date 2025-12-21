#pragma once

#include <memory>
#include <filesystem>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>

namespace SimpleGL {

class Node;
class ShaderProgram;
class Texture;
class MeshComponent;
class MeshData;
class RigidBody;

struct Material2Tex {
    std::shared_ptr<Texture> albedo;
    std::shared_ptr<Texture> specular;
};

class MapLoader {
public:
    static std::shared_ptr<MapLoader> create() {
        return std::make_shared<MapLoader>();
    }

    std::shared_ptr<Node> loadMap(
        const std::filesystem::path& jsonPath,
        const std::shared_ptr<Node>& parent = nullptr
    );

    void addMaterial(const std::string& name, const Material2Tex& material);
    void setDefaultMaterial(const Material2Tex& material);
    void setBlinnPhongShader(const std::shared_ptr<ShaderProgram>& shader);

private:
    std::unordered_map<std::string, Material2Tex> m_materialMap;
    Material2Tex m_defaultMaterial;
    std::shared_ptr<ShaderProgram> m_blinnPhongShader;
    std::shared_ptr<MeshData> m_cubeMeshData;

    std::shared_ptr<Node> createNodeFromJson(
        const nlohmann::json& objData,
        const std::shared_ptr<Node>& parent
    );

    void applyMaterialToMesh(
        const std::shared_ptr<MeshComponent>& mesh,
        const Material2Tex& material
    );

    std::shared_ptr<MeshData> getUnitCube();
};

}