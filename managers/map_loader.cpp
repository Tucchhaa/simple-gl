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

namespace SimpleGL {

std::shared_ptr<MeshData> MapLoader::getUnitCube() {
    if (m_cubeMeshData) {
        return m_cubeMeshData;
    }

    m_cubeMeshData = MeshData::create();
    return m_cubeMeshData;
}


std::shared_ptr<Node> MapLoader::loadMap(
    const std::filesystem::path& jsonPath,
    const std::shared_ptr<Node>& parent
) {
    auto resourcePath = Engine::instance().getResourcePath(jsonPath);

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

    if (!jsonData.contains("objects") || !jsonData["objects"].is_array()) {
        throw std::runtime_error("Map file missing 'objects' array");
    }

    auto rootNode = Node::create("MapRoot", parent);

    int loadedCount = 0;
    for (const auto& objData : jsonData["objects"]) {
        try {
            auto node = createNodeFromJson(objData, rootNode);
            loadedCount++;
        } catch (const std::exception& e) {
            std::cerr << "Error loading object: " << e.what() << std::endl;
        }
    }

    std::cout << "MapLoader: Successfully loaded " << loadedCount << " objects from " << jsonPath << std::endl;

    return rootNode;
}

std::shared_ptr<Node> MapLoader::createNodeFromJson(
    const nlohmann::json& objData,
    const std::shared_ptr<Node>& parent
) {
    if (!objData.contains("name") || !objData.contains("type") || !objData.contains("size")) {
        throw std::runtime_error("Object missing required fields (name, type, size)");
    }

    std::string name = objData["name"];
    std::string type = objData["type"];

    if (type != "box") {
        throw std::runtime_error("Unsupported object type: " + type);
    }

    auto cubeMeshData = getUnitCube();
    auto node = Node::create(name, parent);
    auto meshComponent = MeshComponent::create(node, cubeMeshData);

    auto transform = node->transform();
    
    if (objData.contains("position") && objData["position"].is_array() && objData["position"].size() >= 3) {
        auto pos = objData["position"].get<std::vector<float>>();
        transform->setPosition(pos[0], pos[1], pos[2]);
    }

    if (objData.contains("rotation") && objData["rotation"].is_array() && objData["rotation"].size() >= 4) {
        auto rot = objData["rotation"].get<std::vector<float>>();
        transform->setOrientation(glm::quat(rot[0], rot[1], rot[2], rot[3]));
    }

    if (objData.contains("size") && objData["size"].is_array() && objData["size"].size() >= 3) {
        auto scale = objData["size"].get<std::vector<float>>();
        transform->setScale(scale[0], scale[1], scale[2]);
    }

    transform->recalculate();

    Material2Tex material = m_defaultMaterial;
    if (objData.contains("material")) {
        std::string materialName = objData["material"];
        auto it = m_materialMap.find(materialName);
        if (it != m_materialMap.end()) {
            material = it->second;
        }
    }
    
    if (m_blinnPhongShader && material.albedo && material.specular) {
        applyMaterialToMesh(meshComponent, material);
    }

    if (objData.contains("size") && objData["size"].is_array() && objData["size"].size() >= 3) {
        auto size = objData["size"].get<std::vector<float>>();
        btVector3 halfExtents(size[0] * 0.5f, size[1] * 0.5f, size[2] * 0.5f);
        
        if (halfExtents.x() > 0.0f && halfExtents.y() > 0.0f && halfExtents.z() > 0.0f) {
            auto shape = std::make_shared<btBoxShape>(halfExtents);
            auto rigidBody = RigidBody::create(node);
            rigidBody->setCollisionShape(shape);
            rigidBody->init();
        }
    }

    return node;
}

void MapLoader::addMaterial(const std::string& name, const Material2Tex& material) {
    m_materialMap[name] = material;
}

void MapLoader::setDefaultMaterial(const Material2Tex& material) {
    m_defaultMaterial = material;
}

void MapLoader::setBlinnPhongShader(const std::shared_ptr<ShaderProgram>& shader) {
    m_blinnPhongShader = shader;
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