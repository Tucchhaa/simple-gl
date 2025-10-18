#pragma once

#include <memory>

#include "component.h"

namespace SimpleGL {

class Camera;
class ShaderProgram;
struct MeshData;

class MeshComponent : public Component {
public:
    MeshComponent(
        const std::weak_ptr<Node> &node,
        const std::string &name
    ): Component(node, name) {}

    ~MeshComponent() override;

    static std::shared_ptr<MeshComponent> create(
        const std::shared_ptr<Node> &node,
        const std::shared_ptr<MeshData>& meshData,
        const std::string& name = "Mesh"
    ) {
        auto instance = base_create<MeshComponent>(node, name);

        instance->m_meshData = meshData;
        instance->m_VAO = instance->createVAO();

        return instance;
    }

    unsigned int VAO() const { return m_VAO; }

    void setShader(const std::shared_ptr<ShaderProgram> &shaderProgram);

    void setBeforeDrawCallback(const std::function<void(const std::shared_ptr<ShaderProgram>& shaderProgram)> &beforeDrawCallback) {
        m_beforeDrawCallback = beforeDrawCallback;
    }

    void draw(const std::shared_ptr<Camera>& camera = nullptr) const;

private:
    unsigned int m_VAO = 0;
    unsigned int m_attribOffset = 0;

    std::shared_ptr<MeshData> m_meshData;
    std::shared_ptr<ShaderProgram> m_shaderProgram;

    std::function<void(const std::shared_ptr<ShaderProgram>& shaderProgram)> m_beforeDrawCallback;

    unsigned int createVAO() const;

    void enableVertexAttrib(const std::string& name, bool required, int size);
};


}
