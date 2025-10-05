#include "mesh_data.h"

#include <queue>
#include <glad/glad.h>

namespace SimpleGL {

MeshData::~MeshData() {
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

std::shared_ptr<MeshData> MeshData::createFromScene(const aiScene *scene) {
    auto meshData = parseScene(scene);

    return meshData;
}

std::shared_ptr<MeshData> MeshData::parseScene(const aiScene *scene) {
    auto meshData = MeshData::create();

    std::queue<std::pair<aiNode*, std::shared_ptr<MeshData>>> q;
    q.emplace(scene->mRootNode, meshData);

    while (!q.empty()) {
        const aiNode* currentAiNode = q.front().first;
        const auto currentMeshData = q.front().second;

        q.pop();

        if (currentAiNode->mNumMeshes == 1) {
            const aiMesh* mesh = scene->mMeshes[currentAiNode->mMeshes[0]];
            fillMeshData(mesh, currentMeshData);
        }
        else {
            for (int i=0; i < currentAiNode->mNumMeshes; i++) {
                const aiMesh* mesh = scene->mMeshes[currentAiNode->mMeshes[i]];

                auto subMeshData = MeshData::create();
                currentMeshData->m_subMeshes.push_back(subMeshData);

                fillMeshData(mesh, subMeshData);
            }
        }

        for (int i=0; i < currentAiNode->mNumChildren; i++) {
            auto* node = currentAiNode->mChildren[i];
            auto subMeshData = MeshData::create();

            currentMeshData->m_subMeshes.push_back(subMeshData);
            q.emplace(node, subMeshData);
        }
    }

    return meshData;
}

void MeshData::fillMeshData(const aiMesh *mesh, const std::shared_ptr<MeshData> &meshData) {
    if (mesh->mNumFaces == 0 || mesh->mNumVertices == 0) {
        return;
    }

    const bool hasTextureCoords = mesh->HasTextureCoords(0);

    const int vertexSize = calculateVertexSize(mesh);
    const int numIndicesPerFace = mesh->mFaces[0].mNumIndices;

    const int verticesSize = vertexSize * mesh->mNumVertices;
    const int indicesSize = mesh->mNumFaces * numIndicesPerFace;

    meshData->m_vertices.resize(verticesSize);
    meshData->m_indices.resize(indicesSize);

    for (int i=0; i < mesh->mNumVertices; i++) {
        const int offset = i * vertexSize;

        meshData->m_vertices.at(offset + 0) = mesh->mVertices[i].x;
        meshData->m_vertices.at(offset + 1) = mesh->mVertices[i].y;
        meshData->m_vertices.at(offset + 2) = mesh->mVertices[i].z;

        if (hasTextureCoords) {
            meshData->m_vertices.at(offset + 3) = mesh->mTextureCoords[0][i].x;
            meshData->m_vertices.at(offset + 4) = mesh->mTextureCoords[0][i].y;
        }
    }

    for (int i = 0; i < mesh->mNumFaces; i++) {
        for (int j = 0; j < numIndicesPerFace; j++) {
            const int offset = i * numIndicesPerFace + j;
            meshData->m_indices[offset] = mesh->mFaces[i].mIndices[j];
        }
    }

    createBuffers(meshData);
}

int MeshData::calculateVertexSize(const aiMesh *mesh) {
    int result = 0;

    if (mesh->HasPositions()) {
        constexpr int positionComponent = 3;
        result += positionComponent;
    }

    if (mesh->HasTextureCoords(0)) {
        constexpr int textureComponent = 2;
        result += textureComponent;
    }

    return result;
}

void MeshData::createBuffers(const std::shared_ptr<MeshData> &meshData) {
    glBindVertexArray(0);

    glGenBuffers(1, &meshData->m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, meshData->m_VBO);
    glBufferData(GL_ARRAY_BUFFER, meshData->verticesSize(), meshData->vertices().data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &meshData->m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData->m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshData->indicesSize(), meshData->indices().data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

}
