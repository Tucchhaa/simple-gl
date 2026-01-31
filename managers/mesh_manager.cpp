#include "mesh_manager.h"

#include <queue>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "engine.h"
#include "../entities/node.h"
#include "../entities/components/mesh.h"
#include "../entities/mesh_data.h"
#include "../helpers/errors.h"

namespace SimpleGL {

std::shared_ptr<MeshData> MeshManager::loadMeshData(const std::filesystem::path &path) {
    const auto resourcePath = Engine::get()->getResourcePath(path);

    if (m_meshes.contains(path)) {
        return m_meshes[path];
    }

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        resourcePath.string(),
        aiProcess_RemoveRedundantMaterials | aiProcess_Triangulate
    );

    auto isInvalid = scene == nullptr
                     || scene->mRootNode == nullptr
                     || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE;

    if (isInvalid) {
        throw meshLoadingFailed(resourcePath, importer.GetErrorString());
    }

    const auto meshData = MeshData::createFromScene(scene);

    m_meshes[resourcePath] = meshData;

    return meshData;
}

void MeshManager::freeMeshData(const std::filesystem::path &path) {
    if (m_meshes.contains(path)) {
        m_meshes[path].reset();
    }
}

std::shared_ptr<Node> MeshManager::createNodeFromMeshData(
    const std::filesystem::path& path,
    const std::shared_ptr<Node>& parent
) {
    const auto meshData = loadMeshData(path);
    auto node = Node::create("Node", parent);

    std::queue<std::pair<std::shared_ptr<Node>, std::shared_ptr<MeshData>>> q;
    q.emplace(node, meshData);

    while (!q.empty()) {
        auto [currentNode, currentMeshData] = q.front();

        q.pop();
        currentNode->name = currentMeshData->name();

        if (currentMeshData->hasVertices()) {
            MeshComponent::Factory::create(currentNode, currentMeshData);
        }

        for (const auto& subMeshData : currentMeshData->subMeshes()) {
            auto nextNode = Node::create();

            nextNode->setParent(currentNode);
            q.emplace(nextNode, subMeshData);
        }
    }

    return node;

}

}
