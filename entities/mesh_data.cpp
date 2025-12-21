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

std::shared_ptr<MeshData> MeshData::createFromArrays(
    const std::string& name,
    const std::vector<float>& vertices,
    const std::vector<unsigned int>& indices,
    unsigned int vertexStride
) {
    auto meshData = create();
    meshData->m_name = name;
    meshData->m_vertices = vertices;
    meshData->m_indices = indices;
    
    createBuffers(meshData);
    
    return meshData;
}

std::shared_ptr<MeshData> MeshData::parseScene(const aiScene *scene) {
    auto rootNode = scene->mRootNode;

    while (rootNode->mNumMeshes == 0 && rootNode->mNumChildren == 1) {
        rootNode = rootNode->mChildren[0];
    }

    auto meshData = create();

    std::queue<std::pair<aiNode*, std::shared_ptr<MeshData>>> q;
    q.emplace(rootNode, meshData);

    while (!q.empty()) {
        const auto [currentAiNode, currentMeshData] = q.front();

        q.pop();
        currentMeshData->m_name = currentAiNode->mName.C_Str();

        if (currentAiNode->mNumMeshes == 1) {
            const aiMesh* mesh = scene->mMeshes[currentAiNode->mMeshes[0]];
            fillMeshData(mesh, currentMeshData);
        }
        else {
            for (int i=0; i < currentAiNode->mNumMeshes; i++) {
                const aiMesh* mesh = scene->mMeshes[currentAiNode->mMeshes[i]];

                auto subMeshData = create();
                currentMeshData->m_subMeshes.push_back(subMeshData);

                fillMeshData(mesh, subMeshData);
            }
        }

        for (int i=0; i < currentAiNode->mNumChildren; i++) {
            auto* node = currentAiNode->mChildren[i];
            auto subMeshData = create();

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
    const bool hasNormals = mesh->HasNormals();

    // TODO: support this cases
    if (!hasTextureCoords || !hasNormals) {
        throw std::runtime_error("Mesh does not have normals or texture coords. This case is not supported yet.");
    }

    const unsigned int vertexSize = calculateVertexSize(mesh);
    const unsigned int numIndicesPerFace = mesh->mFaces[0].mNumIndices;

    const unsigned int verticesSize = vertexSize * mesh->mNumVertices;
    const unsigned int indicesSize = mesh->mNumFaces * numIndicesPerFace;

    meshData->m_vertices.resize(verticesSize);
    meshData->m_indices.resize(indicesSize);

    for (int i=0; i < mesh->mNumVertices; i++) {
        const unsigned int offset = i * vertexSize;

        meshData->m_vertices.at(offset + 0) = mesh->mVertices[i].x;
        meshData->m_vertices.at(offset + 1) = mesh->mVertices[i].y;
        meshData->m_vertices.at(offset + 2) = mesh->mVertices[i].z;

        if (hasTextureCoords) {
            meshData->m_vertices.at(offset + 3) = mesh->mTextureCoords[0][i].x;
            meshData->m_vertices.at(offset + 4) = mesh->mTextureCoords[0][i].y;
        }

        if (hasNormals) {
            meshData->m_vertices.at(offset + 5) = mesh->mNormals[i].x;
            meshData->m_vertices.at(offset + 6) = mesh->mNormals[i].y;
            meshData->m_vertices.at(offset + 7) = mesh->mNormals[i].z;
        }
    }

    for (int i = 0; i < mesh->mNumFaces; i++) {
        for (int j = 0; j < numIndicesPerFace; j++) {
            const unsigned int offset = i * numIndicesPerFace + j;
            meshData->m_indices[offset] = mesh->mFaces[i].mIndices[j];
        }
    }

    createBuffers(meshData);
}

unsigned int MeshData::calculateVertexSize(const aiMesh *mesh) {
    unsigned int result = 0;

    if (mesh->HasPositions()) {
        constexpr int positionComponent = 3;
        result += positionComponent;
    }

    if (mesh->HasTextureCoords(0)) {
        constexpr int textureComponent = 2;
        result += textureComponent;
    }

    if (mesh->HasNormals()) {
        constexpr int normalComponent = 3;
        result += normalComponent;
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
