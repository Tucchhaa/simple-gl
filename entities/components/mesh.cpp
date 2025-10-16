#include "mesh.h"

namespace SimpleGL {
MeshComponent::~MeshComponent() {
    glDeleteVertexArrays(1, &m_VAO);
}

void MeshComponent::draw(const std::shared_ptr<Camera> &camera) const {
    if (m_shaderProgram == nullptr) {
        throw meshShaderNotSet(name);
    }

    m_shaderProgram->use(camera);
    m_shaderProgram->setUniform("transform", transform()->transformMatrix());
    m_beforeDrawCallback(m_shaderProgram);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_meshData->indices().size(), GL_UNSIGNED_INT, 0);
}

unsigned int MeshComponent::createVAO() {
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_meshData->VBO());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshData->EBO());

    enableVertexAttrib("vPosition", true, 3);
    enableVertexAttrib("vTextureCoord", false, 2);
    enableVertexAttrib("vNormal", false, 3);

    glBindVertexArray(0);

    return VAO;
}

void MeshComponent::enableVertexAttrib(const std::string &name, bool required, int size) {
    constexpr int positionSize = 3;
    constexpr int textureCoordSize = 2;
    constexpr int normalSize = 3;
    constexpr int stride = (positionSize + textureCoordSize + normalSize) * sizeof(float);

    if (required == false && m_shaderProgram->attribExists(name) == false) {
        m_attribOffset += size * sizeof(float);
        return;
    }

    const int attribLocation = m_shaderProgram->getAttribLocation(name);

    glVertexAttribPointer(attribLocation, size, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(m_attribOffset));
    glEnableVertexAttribArray(attribLocation);

    m_attribOffset += size * sizeof(float);
}

}
