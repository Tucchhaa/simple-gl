#pragma once

#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace SimpleGL {

struct MeshData {
    ~MeshData();

    static std::shared_ptr<MeshData> create() {
        return std::make_shared<MeshData>();
    }

    static std::shared_ptr<MeshData> createFromScene(const aiScene* scene);

    bool hasVertices() const { return !m_vertices.empty(); }

    const std::vector<float>& vertices() { return m_vertices; }
    int verticesSize() const { return sizeof(float) * m_vertices.size(); }

    const std::vector<int>& indices() { return m_indices; }
    int indicesSize() const { return sizeof(float) * m_indices.size(); }

    int VBO() const { return m_VBO; }
    int EBO() const { return m_EBO; }

    const std::vector<std::shared_ptr<MeshData>>& subMeshes() { return m_subMeshes; }

private:
    std::vector<float> m_vertices;

    std::vector<int> m_indices;

    std::vector<std::shared_ptr<MeshData>> m_subMeshes{};

    unsigned int m_VBO = 0;

    unsigned int m_EBO = 0;

    static std::shared_ptr<MeshData> parseScene(const aiScene* scene);

    static void fillMeshData(const aiMesh* mesh, const std::shared_ptr<MeshData>& meshData);

    static int calculateVertexSize(const aiMesh* mesh);

    static void createBuffers(const std::shared_ptr<MeshData>& meshData);
};


}