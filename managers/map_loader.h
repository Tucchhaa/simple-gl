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

// Forward declare Engine for getResourcePath
class Engine;

struct Material2Tex {
    std::shared_ptr<Texture> albedo;
    std::shared_ptr<Texture> specular;
};

class MapLoader {
public:
    static std::shared_ptr<MapLoader> create() {
        return std::make_shared<MapLoader>();
    }

    // Load map from JSON file and create nodes in the scene
    // Returns the root node containing all loaded objects
    std::shared_ptr<Node> loadMap(
        const std::filesystem::path& jsonPath,
        const std::shared_ptr<Node>& parent = nullptr
    );

    // Set materials for texture grouping
    // Prefix mapping: "C_w_" -> wall, "C_f_" -> floor, "C_a_" -> accent, etc.
    void setMaterial(const std::string& prefix, const Material2Tex& material);
    void setDefaultMaterial(const Material2Tex& material);

    // Set shader program to use for textured meshes
    void setBlinnPhongShader(const std::shared_ptr<ShaderProgram>& shader);

    // Get material for a given prefix
    Material2Tex getMaterial(const std::string& prefix) const;

private:
    std::unordered_map<std::string, Material2Tex> m_materialMap;
    Material2Tex m_defaultMaterial;
    std::shared_ptr<ShaderProgram> m_blinnPhongShader;

    // Extract prefix from object name (e.g., "C_w_001" -> "C_w_")
    std::string extractPrefix(const std::string& name) const;

    // Create node from JSON object data
    std::shared_ptr<Node> createNodeFromJson(
        const nlohmann::json& objData,
        const std::shared_ptr<Node>& parent
    );

    // Apply material to mesh component
    void applyMaterialToMesh(
        const std::shared_ptr<MeshComponent>& mesh,
        const Material2Tex& material
    );
};

}

