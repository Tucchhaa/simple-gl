#include <iostream>
#include <sstream>

#include <BulletCollision/CollisionShapes/btBoxShape.h>

#include "managers/engine.h"
#include "managers/mesh_manager.h"
#include "managers/shader_manager.h"
#include "managers/texture_manager.h"
#include "managers/window_manager.h"

#include "entities/input.h"
#include "entities/mesh_data.h"
#include "entities/node.h"
#include "entities/scene.h"
#include "entities/shader_program.h"
#include "entities/window.h"
#include "entities/components/camera.h"
#include "entities/components/free_controller.h"
#include "entities/components/light.h"
#include "entities/components/mesh.h"
#include "entities/components/rigid_body.h"
#include "entities/components/transform.h"
#include "managers/physics_manager.h"

#include "render-pipeline/screen_frame_buffer.h"

using namespace SimpleGL;

int main() {
    auto window = Engine::instance().windowManager()->createWindow("main", 800, 600);

    window->setTitle("Learn OpenGL");

    // create shader
    auto shaderProgram = Engine::instance().shaderManager()->createShaderProgram(
        "shaders/basic/vertex.glsl",
        "shaders/basic/fragment.glsl",
        "basic shader program"
    );

    auto solidColorShaderProgram = Engine::instance().shaderManager()->createShaderProgram(
        "shaders/solid-color/vertex.glsl",
        "shaders/solid-color/fragment.glsl",
        "solid color shader program"
    );

    auto blinnPhongShaderProgram = Engine::instance().shaderManager()->createShaderProgram(
        "shaders/blinn-phong/vertex.glsl",
        "shaders/blinn-phong/fragment.glsl",
        "Blinn-Phong Shader"
    );

    auto blinnPhongSolidColorShaderProgram = Engine::instance().shaderManager()->createShaderProgram(
        "shaders/blinn-phong-solid-color/vertex.glsl",
        "shaders/blinn-phong-solid-color/fragment.glsl",
        "Blinn-Phong Solid Color Shader"
    );

    auto skyboxShaderProgram = Engine::instance().shaderManager()->createShaderProgram(
        "shaders/skybox/vertex.glsl",
        "shaders/skybox/fragment.glsl",
        "Skybox Shader"
    );

    auto frameShaderProgram = Engine::instance().shaderManager()->createShaderProgram(
        "shaders/frame/vertex.glsl",
        "shaders/frame/basic-fragment.glsl",
        "frame shader"
    );

    // create scene
    const auto scene = Scene::create();
    Engine::instance().setScene(scene);

    auto rootNode = Node::create("ROOT");
    scene->setRootNode(rootNode);

    // create skybox
    auto skyboxTexture = Engine::instance().textureManager()->getCubeMapTexture(
        "sky",
        "skybox/right.jpg",
        "skybox/left.jpg",
        "skybox/top.jpg",
        "skybox/bottom.jpg",
        "skybox/front.jpg",
        "skybox/back.jpg",
        false
    );

    auto skyboxNode = Engine::instance().meshManager()->getMesh("cube.obj");
    skyboxNode->setParent(rootNode);
    skyboxNode->name = "skyboxCube";

    auto skyboxCubeMesh = skyboxNode->children().at(0)->getComponent<MeshComponent>();
    skyboxCubeMesh->setShader(skyboxShaderProgram);
    skyboxCubeMesh->setBeforeDrawCallback([&skyboxTexture](const std::shared_ptr<ShaderProgram>& shaderProgram) {
        shaderProgram->setTexture("cubeMap", skyboxTexture);
    });

    // create camera
    auto cameraNode = Node::create("cameraNode");
    cameraNode->setParent(rootNode);
    cameraNode->transform()->setPosition(0, 0, 3);

    auto camera = Camera::create(
        cameraNode,
        glm::radians(45.0f),
        0.3f,
        1000.0f
    );

    auto freeController = FreeController::create(cameraNode);

    // Create cubes
    const auto lightSourceCubeNode = Engine::instance().meshManager()->getMesh("cube.obj");
    lightSourceCubeNode->setParent(rootNode);
    lightSourceCubeNode->name = "lightSourceCube";
    lightSourceCubeNode->transform()->setScale(0.1f);
    lightSourceCubeNode->transform()->setPosition(5, 1, 0);
    lightSourceCubeNode->transform()->setOrientation(glm::quat(glm::radians(glm::vec3(45, -45, 0))));

    const auto lightSourceCubeMesh = lightSourceCubeNode->children().at(0)->getComponent<MeshComponent>();
    lightSourceCubeMesh->setShader(solidColorShaderProgram);
    lightSourceCubeMesh->setBeforeDrawCallback([](const std::shared_ptr<ShaderProgram>& shaderProgram) {
        shaderProgram->setUniform("color", 1, 1, 1);
    });

    // create lights
    const auto pointLight = PointLight::create(lightSourceCubeNode);
    pointLight->ambient = glm::vec3(0.1);
    pointLight->diffuse = glm::vec3(1.0);
    pointLight->specular = glm::vec3(0.4);
    pointLight->distance = 10.0f;

    const auto directLight = DirectLight::create(lightSourceCubeNode);
    directLight->ambient = glm::vec3(0.1);
    directLight->diffuse = glm::vec3(0.7);
    directLight->specular = glm::vec3(0.3);

    // create wooden cube

    const auto cubeNode = Engine::instance().meshManager()->getMesh("cube.obj");
    cubeNode->name = "cube";
    cubeNode->setParent(rootNode);
    cubeNode->transform()->setScale(1);

    const auto cubeMesh = cubeNode->children().at(0)->getComponent<MeshComponent>();

    auto diffuseTexture = Engine::instance().textureManager()->getTexture("diffuse.png");
    auto specularTexture = Engine::instance().textureManager()->getTexture("specular.png");

    cubeMesh->setShader(blinnPhongShaderProgram);
    cubeMesh->setBeforeDrawCallback([&diffuseTexture, &specularTexture](const std::shared_ptr<ShaderProgram>& shaderProgram) {
        shaderProgram->setTexture("diffuseTexture", diffuseTexture);
        shaderProgram->setTexture("specularTexture", specularTexture);
    });

    auto cubeShape = std::make_shared<btBoxShape>(btVector3(0.5f, 0.5f, 0.5f));
    auto cubeRigidBody = RigidBody::create(cubeNode);
    cubeRigidBody->setMass(1.f);
    cubeRigidBody->setCollisionShape(cubeShape);

    // create ground
    const auto groundNode = Engine::instance().meshManager()->getMesh("cube.obj");
    groundNode->name = "ground";
    groundNode->setParent(rootNode);
    groundNode->transform()->setScale(100, 1, 100);
    groundNode->transform()->setPosition(0, -5, 0);

    const auto groundMesh = groundNode->children().at(0)->getComponent<MeshComponent>();

    groundMesh->setShader(blinnPhongSolidColorShaderProgram);
    groundMesh->setBeforeDrawCallback([](const std::shared_ptr<ShaderProgram>& shaderProgram) {
        shaderProgram->setUniform("color", glm::vec3(0.3, 0.3, 0.3));
    });

    auto groundShape = std::make_shared<btBoxShape>(btVector3(50.f, 0.5, 50.f));
    auto groundRigidBody = RigidBody::create(groundNode);
    groundRigidBody->setCollisionShape(groundShape);

    // create screen frame buffer
    const auto screenFrameBuffer = ScreenFrameBuffer::create(window);
    screenFrameBuffer->setShader(frameShaderProgram);

    // rendering pipeline
    window->makeCurrent();

    Engine::instance().physicsManager()->dynamicsWorld()->setGravity(btVector3(0, -10.f, 0));
    scene->emitStart();

    while(window->isOpen())
    {
        Engine::instance().physicsManager()->dynamicsWorld()->stepSimulation(window->input()->deltaTime());

        skyboxNode->transform()->setPosition(cameraNode->transform()->position());

        rootNode->transform()->recalculate();
        camera->recalculateViewMatrix();

        scene->emitUpdate();

        // draw
        glBindFramebuffer(GL_FRAMEBUFFER, screenFrameBuffer->FBO());

        glClearColor(0.1f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        cubeMesh->draw(camera);
        lightSourceCubeMesh->draw(camera);
        groundMesh->draw(camera);

        glCullFace(GL_FRONT);
        glDepthFunc(GL_LEQUAL);
        skyboxCubeMesh->draw(camera);
        glDepthFunc(GL_LESS);

        // draw a quad on the screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        screenFrameBuffer->renderFrame();

        window->afterFrameRendered();

        if (window->input()->isKeyPressed(GLFW_KEY_ESCAPE)) {
            window->close();
        }
    }

    return 0;
}
