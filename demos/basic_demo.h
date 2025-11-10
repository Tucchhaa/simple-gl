#pragma once

#include <BulletCollision/CollisionShapes/btBoxShape.h>

#include "../managers/engine.h"
#include "../managers/mesh_manager.h"
#include "../managers/shader_manager.h"
#include "../managers/texture_manager.h"

#include "../entities/node.h"
#include "../entities/scene.h"
#include "../entities/shader_program.h"
#include "../entities/components/camera.h"
#include "../entities/components/free_controller.h"
#include "../entities/components/light.h"
#include "../entities/components/mesh.h"
#include "../entities/components/rigid_body.h"
#include "../entities/components/transform.h"
#include "../managers/physics_manager.h"

using namespace SimpleGL;

class BasicDemo {
    std::shared_ptr<Node> rootNode;
    std::shared_ptr<MeshManager> meshManager;

    std::shared_ptr<ShaderProgram> basicShader;
    std::shared_ptr<ShaderProgram> solidColorShader;
    std::shared_ptr<ShaderProgram> blinnPhongShader;
    std::shared_ptr<ShaderProgram> blinnPhongSolidColorShader;
    std::shared_ptr<ShaderProgram> skyboxShader;

    std::shared_ptr<Texture> skyboxTexture;
    std::shared_ptr<Texture> cubeDiffuseTexture;
    std::shared_ptr<Texture> cubeSpecularTexture;

    std::shared_ptr<Node> skyboxNode;

    std::shared_ptr<Camera> camera;

    std::shared_ptr<MeshComponent> skyboxCubeMesh;
    std::shared_ptr<MeshComponent> groundMesh;
    std::shared_ptr<MeshComponent> lightSourceCubeMesh;
    std::shared_ptr<MeshComponent> cubeMesh;

public:
    std::shared_ptr<Scene> scene;

    BasicDemo() {
        scene = Scene::create();
        Engine::instance().setScene(scene);

        rootNode = Node::create("ROOT");
        meshManager = Engine::instance().meshManager();
        scene->setRootNode(rootNode);

        Engine::instance().physicsManager()->dynamicsWorld()->setGravity(btVector3(0, -10.f, 0));

        createScene();
    }

    void updateNodes() {
        skyboxNode->transform()->setPosition(camera->transform()->position());
    }

    void draw() {
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        cubeMesh->draw(camera);
        lightSourceCubeMesh->draw(camera);
        groundMesh->draw(camera);

        glCullFace(GL_FRONT);
        glDepthFunc(GL_LEQUAL);
        drawSkybox();
        glDepthFunc(GL_LESS);
    }

    void drawSkybox() {
        skyboxCubeMesh->draw(camera);
    }

private:
    void createScene() {
        createShaders();
        createCamera();
        createSkybox();
        createGround();
        createLightSource();
        createCube();
    }

    void createShaders() {
        basicShader = Engine::instance().shaderManager()->createShaderProgram(
            "shaders/basic/vertex.glsl",
            "shaders/basic/fragment.glsl",
            "basic shader program"
        );

        solidColorShader = Engine::instance().shaderManager()->createShaderProgram(
            "shaders/solid-color/vertex.glsl",
            "shaders/solid-color/fragment.glsl",
            "solid color shader program"
        );

        blinnPhongShader = Engine::instance().shaderManager()->createShaderProgram(
            "shaders/blinn-phong/vertex.glsl",
            "shaders/blinn-phong/fragment.glsl",
            "Blinn-Phong Shader"
        );

        blinnPhongSolidColorShader = Engine::instance().shaderManager()->createShaderProgram(
            "shaders/blinn-phong-solid-color/vertex.glsl",
            "shaders/blinn-phong-solid-color/fragment.glsl",
            "Blinn-Phong Solid Color Shader"
        );

        skyboxShader = Engine::instance().shaderManager()->createShaderProgram(
            "shaders/skybox/vertex.glsl",
            "shaders/skybox/fragment.glsl",
            "Skybox Shader"
        );
    }

    void createCamera() {
        auto cameraNode = Node::create("cameraNode");
        cameraNode->setParent(rootNode);
        cameraNode->transform()->setPosition(0, 0, 3);

        camera = Camera::create(
            cameraNode,
            glm::radians(45.0f),
            0.3f,
            1000.0f
        );

        FreeController::create(cameraNode);
    }

    void createSkybox() {
        skyboxTexture = Engine::instance().textureManager()->getCubeMapTexture(
            "sky",
            "skybox/right.jpg",
            "skybox/left.jpg",
            "skybox/top.jpg",
            "skybox/bottom.jpg",
            "skybox/front.jpg",
            "skybox/back.jpg",
            false
        );

        skyboxNode = meshManager->createNodeFromMeshData("cube.obj");
        skyboxNode->setParent(rootNode);
        skyboxNode->name = "skyboxCube";

        skyboxCubeMesh = skyboxNode->children().at(0)->getComponent<MeshComponent>();
        skyboxCubeMesh->setShader(skyboxShader);
        skyboxCubeMesh->setBeforeDrawCallback([this](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setTexture("cubeMap", skyboxTexture);
        });
    }

    void createGround() {
        const auto groundNode = meshManager->createNodeFromMeshData("cube.obj");
        groundNode->name = "ground";
        groundNode->setParent(rootNode);
        groundNode->transform()->setScale(100, 1, 100);
        groundNode->transform()->setPosition(0, -5, 0);

        groundMesh = groundNode->children().at(0)->getComponent<MeshComponent>();

        groundMesh->setShader(blinnPhongSolidColorShader);
        groundMesh->setBeforeDrawCallback([](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setUniform("color", glm::vec3(0.3, 0.3, 0.3));
        });

        auto groundShape = std::make_shared<btBoxShape>(btVector3(50.f, 0.5, 50.f));
        auto groundRigidBody = RigidBody::create(groundNode);
        groundRigidBody->setCollisionShape(groundShape);
    }

    void createLightSource() {
        const auto lightSourceCubeNode = meshManager->createNodeFromMeshData("cube.obj");
        lightSourceCubeNode->setParent(rootNode);
        lightSourceCubeNode->name = "lightSourceCube";
        lightSourceCubeNode->transform()->setScale(0.1f);
        lightSourceCubeNode->transform()->setPosition(5, 1, 0);
        lightSourceCubeNode->transform()->setOrientation(glm::quat(glm::radians(glm::vec3(45, -45, 0))));

        lightSourceCubeMesh = lightSourceCubeNode->children().at(0)->getComponent<MeshComponent>();
        lightSourceCubeMesh->setShader(solidColorShader);
        lightSourceCubeMesh->setBeforeDrawCallback([](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setUniform("color", 1, 1, 1);
        });

        const auto pointLight = PointLight::create(lightSourceCubeNode);
        pointLight->ambient = glm::vec3(0.1);
        pointLight->diffuse = glm::vec3(1.0);
        pointLight->specular = glm::vec3(0.4);
        pointLight->distance = 10.0f;

        const auto directLight = DirectLight::create(lightSourceCubeNode);
        directLight->ambient = glm::vec3(0.1);
        directLight->diffuse = glm::vec3(0.7);
        directLight->specular = glm::vec3(0.3);
    }

    void createCube() {
        const auto cubeNode = meshManager->createNodeFromMeshData("cube.obj");
        cubeNode->name = "cube";
        cubeNode->setParent(rootNode);
        cubeNode->transform()->setScale(1);

        cubeMesh = cubeNode->children().at(0)->getComponent<MeshComponent>();

        cubeDiffuseTexture = Engine::instance().textureManager()->getTexture("diffuse.png");
        cubeSpecularTexture = Engine::instance().textureManager()->getTexture("specular.png");

        cubeMesh->setShader(blinnPhongShader);
        cubeMesh->setBeforeDrawCallback([this](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setTexture("diffuseTexture", cubeDiffuseTexture);
            shaderProgram->setTexture("specularTexture", cubeSpecularTexture);
        });

        auto cubeShape = std::make_shared<btBoxShape>(btVector3(0.5f, 0.5f, 0.5f));
        auto cubeRigidBody = RigidBody::create(cubeNode);
        cubeRigidBody->setMass(1.f);
        cubeRigidBody->setCollisionShape(cubeShape);
    }
};
