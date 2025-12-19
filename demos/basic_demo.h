#pragma once

#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>

#include "../entities/input.h"
#include "../managers/engine.h"
#include "../managers/mesh_manager.h"
#include "../managers/shader_manager.h"
#include "../managers/texture_manager.h"

#include "../entities/node.h"
#include "../entities/scene.h"
#include "../entities/shader_program.h"
#include "../entities/window.h"
#include "../entities/components/camera.h"
#include "../entities/components/light.h"
#include "../entities/components/mesh.h"
#include "../entities/components/rigid_body.h"
#include "../entities/components/transform.h"
#include "../entities/components/controllers/character_controller.h"
#include "../entities/components/controllers/free_controller.h"
#include "../managers/physics_manager.h"
#include "../managers/window_manager.h"

#include "../render-pipeline/portal/portal.h"

using namespace SimpleGL;

class BasicDemo {
    std::shared_ptr<Node> rootNode;
    std::shared_ptr<MeshManager> meshManager;
    std::shared_ptr<btDynamicsWorld> dynamicsWorld;

    std::shared_ptr<ShaderProgram> basicShader;
    std::shared_ptr<ShaderProgram> solidColorShader;
    std::shared_ptr<ShaderProgram> blinnPhongShader;
    std::shared_ptr<ShaderProgram> shadedSolidColorShader;
    std::shared_ptr<ShaderProgram> skyboxShader;

    std::shared_ptr<Texture> skyboxTexture;
    std::shared_ptr<Texture> cubeDiffuseTexture;
    std::shared_ptr<Texture> cubeSpecularTexture;

    std::shared_ptr<Portal> portal;

    // This node contains static elements of the scene
    std::shared_ptr<Node> staticNode;

    std::shared_ptr<Camera> camera;

    std::vector<std::shared_ptr<MeshComponent>> meshes;
    std::shared_ptr<MeshComponent> skyboxCubeMesh;

public:
    std::shared_ptr<Scene> scene;

    BasicDemo() {
        scene = Scene::create();
        Engine::instance().setScene(scene);

        rootNode = Node::create("ROOT");
        meshManager = Engine::instance().meshManager();
        dynamicsWorld = Engine::instance().physicsManager()->dynamicsWorld();
        scene->setRootNode(rootNode);

        dynamicsWorld->setGravity(btVector3(0, -10.f, 0));

        createScene();
    }

    void updateNodes() {
        float timeStep = Engine::instance().windowManager()->mainWindow()->input()->deltaTime();
        dynamicsWorld->stepSimulation(timeStep);
    }

    void draw() {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // Clear color & depth buffers
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClearColor(0.f, 1.f, 1.f, 1.f);

        glDepthMask(GL_TRUE);
        glClearDepth(1);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // define draw call
        const auto drawCall = [this](const std::shared_ptr<Camera>& _camera) {
            for (const auto& mesh : meshes) {
                mesh->draw(_camera);
            }

            glCullFace(GL_FRONT);
            glDepthFunc(GL_LEQUAL);
            skyboxCubeMesh->draw(_camera);
            glDepthFunc(GL_LESS);
            glCullFace(GL_BACK);
        };

        // draw portals contents
        portal->drawPortal(2, drawCall);
        portal->drawPortal(1, drawCall);

        // draw the rest of scene
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        glDisable(GL_STENCIL_TEST);
        glStencilMask(0x00);

        drawCall(camera);
    }

private:
    void createScene() {
        bool useFreeCamera = false;

        createShaders();

        if (useFreeCamera) {
            createCamera();
        } else {
            createPlayer();
        }

        createSkybox();
        createPortal();

        staticNode = Node::create("staticNode", rootNode);
        createGround();
        createWalls();

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

        shadedSolidColorShader = Engine::instance().shaderManager()->createShaderProgram(
            "shaders/shaded-solid-color/vertex.glsl",
            "shaders/shaded-solid-color/fragment.glsl",
            "Blinn-Phong Solid Color Shader"
        );

        skyboxShader = Engine::instance().shaderManager()->createShaderProgram(
            "shaders/skybox/vertex.glsl",
            "shaders/skybox/fragment.glsl",
            "Skybox Shader"
        );
    }

    void createPlayer() {
        auto playerNode = Node::create("playerNode", rootNode);
        playerNode->transform()->setPosition(0, 0, 3);

        auto playerShape = std::make_shared<btCapsuleShape>(0.35f, 2.5f);
        auto rigidBody = RigidBody::create(playerNode, "playerRigidBody");
        rigidBody->setMass(70.f);
        rigidBody->setCollisionShape(playerShape);
        rigidBody->init();

        auto cameraNode = Node::create("cameraNode", playerNode);
        camera = Camera::create(
            cameraNode,
            glm::radians(90.0f),
            0.3f,
            100.0f
        );

        auto controller = CharacterController::create(playerNode, "playerController");
        controller->setCameraNode(cameraNode);
        controller->setRigidBody(rigidBody);
    }

    void createCamera() {
        auto cameraNode = Node::create("cameraNode", rootNode);
        cameraNode->transform()->setPosition(0, 0, 3);

        camera = Camera::create(
            cameraNode,
            glm::radians(90.0f),
            0.3f,
            100.0f
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

        auto node = meshManager->createNodeFromMeshData("cube.obj", rootNode);
        node->name = "skyboxCube";

        skyboxCubeMesh = node->getComponent<MeshComponent>();
        skyboxCubeMesh->setShader(skyboxShader);
        skyboxCubeMesh->setBeforeDrawCallback([this](const auto& shaderProgram) {
            shaderProgram->setTexture("cubeMap", skyboxTexture);
        });
    }

    void createPortal() {
        auto node = Node::create("portal", rootNode);

        portal = Portal::create(camera);

        // position portals
        portal->portal1Node->transform()->setPosition(-8, -3, -14.45);
        // portal->portal2Node->transform()->setPosition(-1, -3, -14.45);

        portal->portal2Node->transform()->setPosition(-8, -3, -10);
        portal->portal2Node->transform()->rotate(glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 1.0f, 0.0f)));
        portal->portal2Node->transform()->rotate(glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 1.0f, 0.0f)));

        const auto planeMesh = meshManager->loadMeshData("plane.obj");
        portal->setPortalMesh(1, planeMesh);
        portal->setPortalMesh(2, planeMesh);

        glm::quat orientation = glm::quat(1, 0, 0, 0);

        orientation = glm::angleAxis(glm::radians(90.f), glm::vec3(1.0f, 0, 0)) * orientation;
        // orientation = glm::angleAxis(glm::radians(180.f), glm::vec3(0, 0, 1)) * orientation;
        portal->portal1Node->getChild("childNode")->transform()->setOrientation(orientation);
        portal->portal2Node->getChild("childNode")->transform()->setOrientation(orientation);

        auto portal1BorderNode = portal->portal1Node->getChild("childNode")->getChild("borderNode");
        portal1BorderNode->transform()->scaleBy(1.05);

        auto portal1BorderMesh = portal1BorderNode->getComponent<MeshComponent>();
        portal1BorderMesh->setShader(solidColorShader);
        portal1BorderMesh->setBeforeDrawCallback([](const auto& shader) {
            shader->setUniform("color", 0.05f, 0.15f, 1.f);
        });

        auto portal2BorderNode = portal->portal2Node->getChild("childNode")->getChild("borderNode");
        portal2BorderNode->transform()->scaleBy(1.05);

        auto portal2BorderMesh = portal2BorderNode->getComponent<MeshComponent>();
        portal2BorderMesh->setShader(solidColorShader);
        portal2BorderMesh->setBeforeDrawCallback([](const auto& shader) {
            shader->setUniform("color", 1.f, 0.05f, 0.05f);
        });
    }

    void createGround() {
        const auto node = meshManager->createNodeFromMeshData("cube.obj", staticNode);
        node->name = "ground";
        node->transform()->setScale(100, 1, 100);
        node->transform()->setPosition(0, -5, 0);

        auto mesh = node->getComponent<MeshComponent>();
        mesh->setShader(shadedSolidColorShader);
        mesh->setBeforeDrawCallback([](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setUniform("color", glm::vec3(0.3, 0.3, 0.3));
        });

        meshes.push_back(mesh);

        auto groundShape = std::make_shared<btBoxShape>(btVector3(50.f, 0.5, 50.f));
        auto rigidbody = RigidBody::create(node);
        rigidbody->setCollisionShape(groundShape);
        rigidbody->init();
    }

    void createWalls() {
        float scale = 30.0f;
        float positionX[] = { 0.0f, 0.5f, 0.0f, -0.5f };
        float positionZ[] = { 0.5f, 0.0f, -0.5f, 0.0f };

        for (int i=0; i < 4; i++) {
            auto node = meshManager->createNodeFromMeshData("cube.obj", staticNode);
            node->name = "wallNode"+std::to_string(i);
            node->transform()->setScale(scale, 4, 1);
            node->transform()->setPosition(
                positionX[i] * scale,
                -3.0f,
                positionZ[i] * scale
            );

            if (positionX[i] != 0.0f) {
                auto r = glm::angleAxis(glm::radians(90.f), glm::vec3(0, 1, 0));
                node->transform()->rotate(r);
            }

            auto mesh = node->getComponent<MeshComponent>();
            mesh->setShader(shadedSolidColorShader);
            mesh->setBeforeDrawCallback([](const std::shared_ptr<ShaderProgram>& shaderProgram) {
                shaderProgram->setUniform("color", glm::vec3(0.3, 0.3, 0.3));
            });

            meshes.push_back(mesh);
        }
    }

    void createLightSource() {
        const auto node = meshManager->createNodeFromMeshData("cube.obj", rootNode);
        node->name = "lightSourceCube";
        node->transform()->setScale(0.1f);
        node->transform()->setPosition(5, 1, 0);
        node->transform()->setOrientation(glm::quat(glm::radians(glm::vec3(45, -45, 0))));

        auto mesh = node->getComponent<MeshComponent>();
        mesh->setShader(solidColorShader);
        mesh->setBeforeDrawCallback([](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setUniform("color", 1, 1, 1);
        });

        meshes.push_back(mesh);

        const auto pointLight = PointLight::create(node);
        pointLight->ambient = glm::vec3(0.0);
        pointLight->diffuse = glm::vec3(0.7);
        pointLight->specular = glm::vec3(0.2);
        pointLight->distance = 10.0f;

        const auto directLight = DirectLight::create(node);
        directLight->ambient = glm::vec3(0.05);
        directLight->diffuse = glm::vec3(0.25);
        directLight->specular = glm::vec3(0.04);
    }

    void createCube() {
        const auto node = meshManager->createNodeFromMeshData("cube.obj", rootNode);
        node->name = "cube";
        node->transform()->setScale(1);

        auto mesh = node->getComponent<MeshComponent>();

        cubeDiffuseTexture = Engine::instance().textureManager()->getTexture("diffuse.png", true);
        cubeSpecularTexture = Engine::instance().textureManager()->getTexture("specular.png", false);

        mesh->setShader(blinnPhongShader);
        mesh->setBeforeDrawCallback([this](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setTexture("diffuseTexture", cubeDiffuseTexture);
            shaderProgram->setTexture("specularTexture", cubeSpecularTexture);
        });
        meshes.push_back(mesh);

        auto cubeShape = std::make_shared<btBoxShape>(btVector3(0.5f, 0.5f, 0.5f));
        auto rigidBody = RigidBody::create(node);
        rigidBody->setMass(1.f);
        rigidBody->setCollisionShape(cubeShape);
        rigidBody->init();
    }
};
