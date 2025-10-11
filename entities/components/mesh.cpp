#include "mesh.h"

namespace SimpleGL {
MeshComponent::~MeshComponent() {
    glDeleteVertexArrays(1, &m_VAO);
}

void MeshComponent::draw(const std::shared_ptr<Camera> &camera) const {
    if (m_shaderProgram == nullptr) {
        throw meshShaderNotSet(name);
    }

    m_shaderProgram->use();

    m_shaderProgram->setUniform("transform", transform()->transformMatrix());
    m_shaderProgram->setUniform("view", camera->viewMatrix());
    m_shaderProgram->setUniform("projection", camera->projectionMatrix());

    m_beforeDrawCallback(m_shaderProgram);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_meshData->indices().size(), GL_UNSIGNED_INT, 0);
}

unsigned int MeshComponent::createVAO() const {
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_meshData->VBO());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshData->EBO());

    const int positionAttrib = m_shaderProgram->getAttribLocation("vPosition");
    glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(positionAttrib);

    const int textureCoordAttrib = m_shaderProgram->getAttribLocation("vTextureCoord", false);

    if (textureCoordAttrib != -1) {
        glVertexAttribPointer(textureCoordAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(textureCoordAttrib);
    }

    glBindVertexArray(0);

    return VAO;
}

}
