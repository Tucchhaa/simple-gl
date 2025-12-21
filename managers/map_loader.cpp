#include "map_loader.h"

#include <fstream>
#include <stdexcept>
#include <iostream>

#include <BulletCollision/CollisionShapes/btBoxShape.h>

#include "engine.h"
#include "../entities/node.h"
#include "../entities/components/mesh.h"
#include "../entities/components/transform.h"
#include "../entities/components/rigid_body.h"
#include "../entities/mesh_data.h"
#include "../entities/shader_program.h"
#include "../helpers/errors.h"

namespace SimpleGL {

std::shared_ptr<Node> MapLoader::loadMap(
    const std::filesystem::path& jsonPath,
    const std::shared_ptr<Node>& parent
) {
    // Get full resource path
    auto resourcePath = Engine::instance().getResourcePath(jsonPath);

    // Read JSON file
    std::ifstream file(resourcePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open map file: " + resourcePath.string());
    }

    nlohmann::json jsonData;
    try {
        file >> jsonData;
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }

    // Validate version
    if (!jsonData.contains("version") || jsonData["version"] != 1) {
        throw std::runtime_error("Unsupported map format version");
    }

    if (!jsonData.contains("objects") || !jsonData["objects"].is_array()) {
        throw std::runtime_error("Map file missing 'objects' array");
    }

    // Create root node for all map objects
    auto rootNode = Node::create("MapRoot", parent);

    // Process each object
    int loadedCount = 0;
    for (const auto& objData : jsonData["objects"]) {
        try {
            auto node = createNodeFromJson(objData, rootNode);
            loadedCount++;
        } catch (const std::exception& e) {
            // Log error but continue loading other objects
            std::cerr << "Error loading object: " << e.what() << std::endl;
        }
    }

    std::cout << "MapLoader: Successfully loaded " << loadedCount << " objects from " << jsonPath << std::endl;

    return rootNode;
}

void MapLoader::setMaterial(const std::string& prefix, const Material2Tex& material) {
    m_materialMap[prefix] = material;
}

void MapLoader::setDefaultMaterial(const Material2Tex& material) {
    m_defaultMaterial = material;
}

void MapLoader::setBlinnPhongShader(const std::shared_ptr<ShaderProgram>& shader) {
    m_blinnPhongShader = shader;
}

Material2Tex MapLoader::getMaterial(const std::string& prefix) const {
    auto it = m_materialMap.find(prefix);
    if (it != m_materialMap.end()) {
        return it->second;
    }
    return m_defaultMaterial;
}

std::string MapLoader::extractPrefix(const std::string& name) const {
    // Extract prefix up to the last underscore (e.g., "C_w_001" -> "C_w_")
    size_t lastUnderscore = name.find_last_of('_');
    if (lastUnderscore != std::string::npos && lastUnderscore > 0) {
        return name.substr(0, lastUnderscore + 1);
    }
    return "";
}

std::shared_ptr<Node> MapLoader::createNodeFromJson(
    const nlohmann::json& objData,
    const std::shared_ptr<Node>& parent
) {
    // Extract required fields
    if (!objData.contains("name") || !objData.contains("vertices") || !objData.contains("indices")) {
        throw std::runtime_error("Object missing required fields (name, vertices, indices)");
    }

    std::string name = objData["name"];
    
    // Extract vertex data
    std::vector<float> vertices = objData["vertices"].get<std::vector<float>>();
    std::vector<unsigned int> indices = objData["indices"].get<std::vector<unsigned int>>();

    if (vertices.empty() || indices.empty()) {
        throw std::runtime_error("Object has empty vertices or indices: " + name);
    }

    // Create mesh data from arrays
    auto meshData = MeshData::createFromArrays(name, vertices, indices, 8);

    // Create node
    auto node = Node::create(name, parent);

    // Create mesh component
    auto meshComponent = MeshComponent::create(node, meshData);

    // Apply transform
    auto transform = node->transform();
    
    if (objData.contains("position") && objData["position"].is_array() && objData["position"].size() >= 3) {
        auto pos = objData["position"].get<std::vector<float>>();
        transform->setPosition(pos[0], pos[1], pos[2]);
    }

    if (objData.contains("rotation") && objData["rotation"].is_array() && objData["rotation"].size() >= 4) {
        auto rot = objData["rotation"].get<std::vector<float>>();
        transform->setOrientation(rot[0], rot[1], rot[2], rot[3]);  // w, x, y, z
    }

    if (objData.contains("scale")) {
        if (objData["scale"].is_number()) {
            float scale = objData["scale"];
            transform->setScale(scale);
        } else if (objData["scale"].is_array() && objData["scale"].size() >= 3) {
            auto scale = objData["scale"].get<std::vector<float>>();
            transform->setScale(scale[0], scale[1], scale[2]);
        }
    }

    // Force transform recalculation to ensure matrix is correct
    transform->recalculate();

    // Apply default material to all objects
    Material2Tex material = m_defaultMaterial;
    
    if (m_blinnPhongShader && material.albedo && material.specular) {
        applyMaterialToMesh(meshComponent, material);
    }

    // Create collision body if half-extents are provided
    if (objData.contains("colliderHalfExtents") && 
        objData["colliderHalfExtents"].is_array() && 
        objData["colliderHalfExtents"].size() >= 3) {
        
        auto halfExtents = objData["colliderHalfExtents"].get<std::vector<float>>();
        
        // Only create collider if extents are non-zero
        if (halfExtents[0] > 0.0f && halfExtents[1] > 0.0f && halfExtents[2] > 0.0f) {
            auto shape = std::make_shared<btBoxShape>(
                btVector3(halfExtents[0], halfExtents[1], halfExtents[2])
            );
            auto rigidBody = RigidBody::create(node);
            rigidBody->setCollisionShape(shape);
            rigidBody->init();
        }
    }

    return node;
}

void MapLoader::applyMaterialToMesh(
    const std::shared_ptr<MeshComponent>& mesh,
    const Material2Tex& material
) {
    if (!m_blinnPhongShader) {
        return;
    }

    mesh->setShader(m_blinnPhongShader);
    mesh->setBeforeDrawCallback([material](const std::shared_ptr<ShaderProgram>& shader) {
        shader->setTexture("diffuseTexture", material.albedo);
        shader->setTexture("specularTexture", material.specular);
    });
}

}

