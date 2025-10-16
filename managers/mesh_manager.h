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

    std::shared_ptr<Node> getMesh(const std::filesystem::path& path);

private:
    std::unordered_map<std::string, std::weak_ptr<MeshData>> m_meshes;

    static std::shared_ptr<Node> createNode(const std::shared_ptr<MeshData>& meshData);
};

}
