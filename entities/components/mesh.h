#pragma once

#include <memory>
#include <glad/glad.h>

#include "camera.h"
#include "component.h"
#include "transform.h"
#include "../mesh_data.h"
#include "../shader_program.h"
#include "../../helpers/errors.h"

namespace SimpleGL {

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
        return instance;
    }

    unsigned int VAO() const { return m_VAO; }

    void setShader(const std::shared_ptr<ShaderProgram> &shaderProgram) {
        m_shaderProgram = shaderProgram;
        m_VAO = createVAO();
    }

    void setBeforeDrawCallback(const std::function<void(const std::shared_ptr<ShaderProgram>& shaderProgram)> &beforeDrawCallback) {
        m_beforeDrawCallback = beforeDrawCallback;
    }

    void draw(const std::shared_ptr<Camera>& camera) const;

private:
    unsigned int m_VAO = 0;

    std::shared_ptr<MeshData> m_meshData;

    std::shared_ptr<ShaderProgram> m_shaderProgram;

    std::function<void(const std::shared_ptr<ShaderProgram>& shaderProgram)> m_beforeDrawCallback;

    unsigned int m_attribOffset = 0;

    unsigned int createVAO();

    void enableVertexAttrib(const std::string& name, bool required, int size);
};


}
