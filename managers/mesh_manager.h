#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <filesystem>

namespace SimpleGL {

class Node;
class Component;
struct MeshData;

class MeshManager {
public:
    static std::shared_ptr<MeshManager> create() {
        return std::make_shared<MeshManager>();
    }

    std::shared_ptr<MeshData> loadMeshData(const std::filesystem::path& path);

    void freeMeshData(const std::filesystem::path& path);

    std::shared_ptr<Node> createNodeFromMeshData(
        const std::filesystem::path& path,
        const std::shared_ptr<Node>& parent = nullptr
    );

private:
    std::unordered_map<std::string, std::shared_ptr<MeshData>> m_meshes;
};

}
