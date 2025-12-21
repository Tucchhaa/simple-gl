#include "map_loader.h"

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <nlohmann/json.hpp>

#include <BulletCollision/CollisionShapes/btBoxShape.h>

#include "engine.h"
#include "../entities/node.h"
#include "../entities/components/mesh.h"
#include "../entities/components/transform.h"
#include "../entities/components/rigid_body.h"
#include "../entities/mesh_data.h"
#include "../entities/shader_program.h"
#include "../helpers/errors.h"

// #region agent log
static void debug_log(const std::string& location, const std::string& message, const nlohmann::json& data, const std::string& hypothesisId = "") {
    nlohmann::json log_entry = {
        {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()},
        {"location", location},
        {"message", message},
        {"data", data},
        {"sessionId", "debug-session"},
        {"runId", "run1"},
        {"hypothesisId", hypothesisId}
    };
    std::ofstream log_file("/home/franciscobrizuela/Documents/codes1/simple-gl/.cursor/debug.log", std::ios::app);
    log_file << log_entry.dump() << "\n";
    log_file.close();
}
// #endregion

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

    // #region agent log
    int totalObjects = jsonData["objects"].size();
    debug_log("map_loader.cpp:50", "Starting map load", {
        {"totalObjects", totalObjects},
        {"jsonPath", jsonPath.string()}
    }, "A");
    // #endregion

    // Process each object
    int loadedCount = 0;
    for (const auto& objData : jsonData["objects"]) {
        try {
            auto node = createNodeFromJson(objData, rootNode);
            loadedCount++;
            // #region agent log
            debug_log("map_loader.cpp:62", "Object loaded", {
                {"objectName", objData["name"].get<std::string>()},
                {"loadedCount", loadedCount}
            }, "A");
            // #endregion
        } catch (const std::exception& e) {
            // Log error but continue loading other objects
            std::cerr << "Error loading object: " << e.what() << std::endl;
            // #region agent log
            debug_log("map_loader.cpp:69", "Object load failed", {
                {"error", e.what()}
            }, "A");
            // #endregion
        }
    }

    std::cout << "MapLoader: Successfully loaded " << loadedCount << " objects from " << jsonPath << std::endl;

    // #region agent log
    debug_log("map_loader.cpp:77", "Map load complete", {
        {"loadedCount", loadedCount},
        {"totalObjects", totalObjects}
    }, "A");
    // #endregion

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
    if (!objData.contains("name") || !objData.contains("type") || !objData.contains("size")) {
        throw std::runtime_error("Object missing required fields (name, type, size)");
    }

    std::string name = objData["name"];
    std::string type = objData["type"];

    if (type != "box") {
        throw std::runtime_error("Unsupported object type: " + type + ". Only 'box' is supported.");
    }

    // Get the unit cube mesh data (or create it if it doesn't exist)
    auto cubeMeshData = getUnitCube();

    // Create node
    auto node = Node::create(name, parent);

    // Create mesh component using the shared cube mesh
    auto meshComponent = MeshComponent::create(node, cubeMeshData);

    // #region agent log
    debug_log("map_loader.cpp:143", "Mesh component created from unit cube", {
        {"objectName", name}
    }, "B");
    // #endregion

    // Apply transform
    auto transform = node->transform();
    
    if (objData.contains("position") && objData["position"].is_array() && objData["position"].size() >= 3) {
        auto pos = objData["position"].get<std::vector<float>>();
        transform->setPosition(pos[0], pos[1], pos[2]);
    }

    if (objData.contains("rotation") && objData["rotation"].is_array() && objData["rotation"].size() >= 4) {
        auto rot = objData["rotation"].get<std::vector<float>>();
        // JSON has w, x, y, z. GLM constructor is w, x, y, z.
        transform->setOrientation(glm::quat(rot[0], rot[1], rot[2], rot[3]));
    }

    // Use "size" for scale
    if (objData.contains("size") && objData["size"].is_array() && objData["size"].size() >= 3) {
        auto scale = objData["size"].get<std::vector<float>>();
        transform->setScale(scale[0], scale[1], scale[2]);
    }

    // Force transform recalculation to ensure matrix is correct
    transform->recalculate();

    // Apply default material to all objects
    Material2Tex material = m_defaultMaterial;
    
    // #region agent log
    auto absPos = transform->absolutePosition();
    debug_log("map_loader.cpp:200", "Transform applied", {
        {"objectName", name},
        {"absolutePosition", {absPos.x, absPos.y, absPos.z}},
        {"hasShader", m_blinnPhongShader != nullptr},
        {"hasAlbedo", material.albedo != nullptr},
        {"hasSpecular", material.specular != nullptr}
    }, "C");
    // #endregion
    
    if (m_blinnPhongShader && material.albedo && material.specular) {
        applyMaterialToMesh(meshComponent, material);
        // #region agent log
        debug_log("map_loader.cpp:211", "Material applied", {
            {"objectName", name}
        }, "D");
        // #endregion
    } else {
        // #region agent log
        std::string reason = !m_blinnPhongShader ? "no_shader" : (!material.albedo || !material.specular ? "no_textures" : "unknown");
        debug_log("map_loader.cpp:216", "Material NOT applied", {
            {"objectName", name},
            {"reason", reason}
        }, "D");
        // #endregion
    }

    // Create collision body using the object's size
    if (objData.contains("size") && 
        objData["size"].is_array() && 
        objData["size"].size() >= 3) {
        
        auto size = objData["size"].get<std::vector<float>>();
        
        // Half-extents are half of the size
        btVector3 halfExtents(size[0] * 0.5f, size[1] * 0.5f, size[2] * 0.5f);
        
        // Only create collider if extents are non-zero
        if (halfExtents.x() > 0.0f && halfExtents.y() > 0.0f && halfExtents.z() > 0.0f) {
            auto shape = std::make_shared<btBoxShape>(halfExtents);
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

std::shared_ptr<MeshData> MapLoader::getUnitCube() {
    // If the cube mesh data has already been created, return it.
    if (m_cubeMeshData) {
        return m_cubeMeshData;
    }

    // Define the vertices for a 1x1x1 cube.
    // Each vertex has position (3), normal (3), and UV (2) data.
    // Total = 8 floats per vertex.
    std::vector<float> vertices = {
        // Front face (+Z)
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
        // Back face (-Z)
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
        // Left face (-X)
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        // Right face (+X)
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        // Top face (+Y)
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
        // Bottom face (-Y)
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f
    };

    std::vector<unsigned int> indices = {
        0, 1, 2, 0, 2, 3,       // Front
        4, 5, 6, 4, 6, 7,       // Back
        8, 9, 10, 8, 10, 11,    // Left
        12, 13, 14, 12, 14, 15, // Right
        16, 17, 18, 16, 18, 19, // Top
        20, 21, 22, 20, 22, 23  // Bottom
    };

    // Create the mesh data and cache it.
    // The stride is 8 floats (3 pos, 3 normal, 2 uv).
    m_cubeMeshData = MeshData::createFromArrays("unit_cube", vertices, indices, 8);
    return m_cubeMeshData;
}

}

