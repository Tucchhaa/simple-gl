#pragma once

#include <memory>
#include <glad/glad.h>

#include "component.h"
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

    ~MeshComponent() override {
        glDeleteVertexArrays(1, &m_VAO);
    }

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

    void draw() const {
        if (m_shaderProgram == nullptr) {
            throw meshShaderNotSet(name);
        }

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, m_meshData->indices().size(), GL_UNSIGNED_INT, 0);
    }

private:
    unsigned int m_VAO = 0;

    std::shared_ptr<MeshData> m_meshData;

    std::shared_ptr<ShaderProgram> m_shaderProgram;

    unsigned int createVAO() const {
        unsigned int VAO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_meshData->VBO());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshData->EBO());

        const int positionAttrib = m_shaderProgram->getAttribLocation("vPosition");
        glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(positionAttrib);

        const int textureCoordAttrib = m_shaderProgram->getAttribLocation("vTextureCoord");
        glVertexAttribPointer(textureCoordAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(textureCoordAttrib);

        glBindVertexArray(0);

        return VAO;
    }
};


}
