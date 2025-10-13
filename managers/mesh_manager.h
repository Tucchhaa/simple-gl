#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <filesystem>

#include "../entities/components/component.h"
#include "../entities/mesh_data.h"

namespace SimpleGL {

class MeshManager {
public:
    static std::shared_ptr<MeshManager> create() {
        return std::make_shared<MeshManager>();
    }

    std::shared_ptr<Node> getMesh(const std::filesystem::path& path, const std::string& name);

private:
    std::unordered_map<std::string, std::shared_ptr<MeshData>> m_meshes;

    static std::shared_ptr<Node> createNode(const std::string &name, const std::shared_ptr<MeshData>& meshData);
};

}
