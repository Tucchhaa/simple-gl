#include "mesh.h"

#include "camera.h"
#include "transform.h"
#include "../mesh_data.h"
#include "../node.h"
#include "../shader_program.h"
#include "../../helpers/errors.h"

// #region agent log
#include <fstream>
#include <chrono>
#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// #endregion

namespace SimpleGL {

MeshComponent::~MeshComponent() {
    glDeleteVertexArrays(1, &m_VAO);
}

std::shared_ptr<MeshComponent> MeshComponent::create(
    const std::shared_ptr<Node> &node,
    const std::shared_ptr<MeshData> &meshData,
    const std::string &name
) {
    auto instance = base_create<MeshComponent>(node, name);

    instance->m_meshData = meshData;
    instance->m_VAO = instance->createVAO();

    return instance;
}

void MeshComponent::setShader(const std::shared_ptr<ShaderProgram> &shaderProgram) {
    m_shaderProgram = shaderProgram;

    glBindVertexArray(m_VAO);
    enableVertexAttrib("vPosition", true, 3);
    enableVertexAttrib("vTextureCoord", false, 2);
    enableVertexAttrib("vNormal", false, 3);
    glBindVertexArray(0);
    m_attribOffset = 0;
}

void MeshComponent::draw(const std::shared_ptr<Camera>& camera) const {
    if (node()->visible == false) {
        return;
    }

    if (m_shaderProgram == nullptr) {
        throw meshShaderNotSet(name);
    }

    m_shaderProgram->use(camera);

    if (m_shaderProgram->uniformExists("transform")) {
        auto tMatrix = transform()->transformMatrix();
        // #region agent log
        auto camPos = camera->transform()->absolutePosition();
        auto camDir = camera->transform()->direction();
        std::ofstream log_file("/home/franciscobrizuela/Documents/codes1/simple-gl/.cursor/debug.log", std::ios::app);
        nlohmann::json log_entry = {
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()},
            {"location", "mesh.cpp:52"},
            {"message", "Drawing mesh with camera context"},
            {"data", {
                {"nodeName", node()->name},
                {"cameraPosition", {camPos.x, camPos.y, camPos.z}},
                {"cameraDirection", {camDir.x, camDir.y, camDir.z}},
                {"objectPosition", {tMatrix[3][0], tMatrix[3][1], tMatrix[3][2]}},
                {"distanceFromCamera", glm::distance(camPos, glm::vec3(tMatrix[3][0], tMatrix[3][1], tMatrix[3][2]))}
            }},
            {"sessionId", "debug-session"},
            {"runId", "run1"},
            {"hypothesisId", "G"}
        };
        log_file << log_entry.dump() << "\n";
        log_file.close();
        // #endregion
        m_shaderProgram->setUniform("transform", tMatrix);
    }

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
