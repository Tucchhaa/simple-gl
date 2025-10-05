#include "mesh_manager.h"

#include <queue>
#include "engine.h"
#include "../entities/components/mesh.h"
#include "../helpers/errors.h"

#include <assimp/postprocess.h>

namespace SimpleGL {

std::shared_ptr<Node> MeshManager::loadMesh(const std::filesystem::path &path, const std::string &name) {
    const auto resourcePath = Engine::instance().getResourcePath(path);

    if (auto cachedMesh = m_meshes.find(resourcePath); cachedMesh != m_meshes.end()) {
        return createNode(name, cachedMesh->second);
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

    return createNode(name, meshData);
}

std::shared_ptr<Node> MeshManager::createNode(const std::string &name, const std::shared_ptr<MeshData> &meshData) {
    auto node = Node::create(name);

    std::queue<std::pair<std::shared_ptr<Node>, std::shared_ptr<MeshData>>> q;
    q.emplace(node, meshData);

    while (!q.empty()) {
        auto currentNode = q.front().first;
        auto currentMeshData = q.front().second;
        q.pop();

        if (currentMeshData->hasVertices()) {
            MeshComponent::create(currentNode, currentMeshData);
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
