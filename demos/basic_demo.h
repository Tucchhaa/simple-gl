#pragma once

#include <BulletCollision/CollisionShapes/btBoxShape.h>

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
#include "../entities/components/free_controller.h"
#include "../entities/components/light.h"
#include "../entities/components/mesh.h"
#include "../entities/components/portal.h"
#include "../entities/components/rigid_body.h"
#include "../entities/components/transform.h"
#include "../managers/physics_manager.h"
#include "../managers/window_manager.h"

using namespace SimpleGL;

class BasicDemo {
    std::shared_ptr<Node> rootNode;
    std::shared_ptr<MeshManager> meshManager;
    std::shared_ptr<btDynamicsWorld> dynamicsWorld;

    std::shared_ptr<ShaderProgram> basicShader;
    std::shared_ptr<ShaderProgram> solidColorShader;
    std::shared_ptr<ShaderProgram> blinnPhongShader;
    std::shared_ptr<ShaderProgram> blinnPhongSolidColorShader;
    std::shared_ptr<ShaderProgram> skyboxShader;

    std::shared_ptr<Texture> skyboxTexture;
    std::shared_ptr<Texture> cubeDiffuseTexture;
    std::shared_ptr<Texture> cubeSpecularTexture;

    // This node contains static elements of the scene
    std::shared_ptr<Node> staticNode;

    std::shared_ptr<Camera> camera;
    std::shared_ptr<Portal> portal;

    std::vector<std::shared_ptr<MeshComponent>> meshes;
    std::shared_ptr<MeshComponent> skyboxCubeMesh;
    std::shared_ptr<MeshComponent> portal1Mesh;
    std::shared_ptr<MeshComponent> portal2Mesh;

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

        // Clear all buffers
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
        glStencilMask(0xFF);
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClearStencil(0);
        glClearDepth(1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Draw portal into stencil buffer
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        portal1Mesh->draw(camera);

        // Draw scene from virtual camera into the portal frame
        glEnable(GL_DEPTH_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
        glStencilMask(0x00);
        glStencilFunc(GL_EQUAL, 1, 0xFF);

        for (const auto& mesh : meshes) {
            mesh->draw(portal->portal2VirtualCamera);
        }

        drawSkybox(portal->portal2VirtualCamera);

        // Draw portal to depth buffer
        glDisable(GL_STENCIL_TEST);
        glClear(GL_DEPTH_BUFFER_BIT);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        portal1Mesh->draw(camera);

        // Draw scene from main camera
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        for (const auto& mesh: meshes) {
            mesh->draw(camera);
        }

        portal2Mesh->draw(camera);

        drawSkybox(camera);
    }

    void drawSkybox(const std::shared_ptr<Camera>& _camera) {
        glCullFace(GL_FRONT);
        glDepthFunc(GL_LEQUAL);
        skyboxCubeMesh->draw(_camera);
        glDepthFunc(GL_LESS);
        glCullFace(GL_BACK);
    }

private:
    void createScene() {
        createShaders();
        createCamera();
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
        auto cameraNode = Node::create("cameraNode", rootNode);
        cameraNode->transform()->setPosition(0, 0, 3);

        camera = Camera::create(
            cameraNode,
            glm::radians(90.0f),
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

        portal = Portal::create(node, camera);

        // Need to rotate plane such that its normal is parallel to direction vector of parent node
        const auto orientation = glm::angleAxis(glm::radians(90.f), glm::vec3(1.0f, 0, 0));
        portal->portal1Node->getChild("meshNode")->transform()->setOrientation(orientation);
        portal->portal2Node->getChild("meshNode")->transform()->setOrientation(orientation);

        portal->portal1Node->transform()->setPosition(3, -3, -14.45);
        portal->portal2Node->transform()->setPosition(-1, -3, -14.45);

        const auto planeMesh = meshManager->loadMeshData("plane.obj");
        portal->setPortalMesh(planeMesh);

        portal1Mesh = portal->portal1Node->getChildComponent<MeshComponent>();
        portal2Mesh = portal->portal2Node->getChildComponent<MeshComponent>();

        portal1Mesh->setShader(solidColorShader);
        portal1Mesh->setBeforeDrawCallback([](const auto& shader) {
            shader->setUniform("color", 0.5f, 0.1f, 0.1f);
        });

        portal2Mesh->setShader(solidColorShader);
        portal2Mesh->setBeforeDrawCallback([](const auto& shader) {
            shader->setUniform("color", 0.1f, 0.5f, 0.1f);
        });
    }

    void createGround() {
        const auto node = meshManager->createNodeFromMeshData("cube.obj", staticNode);
        node->name = "ground";
        node->transform()->setScale(100, 1, 100);
        node->transform()->setPosition(0, -5, 0);

        auto mesh = node->getComponent<MeshComponent>();
        mesh->setShader(blinnPhongSolidColorShader);
        mesh->setBeforeDrawCallback([](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setUniform("color", glm::vec3(0.3, 0.3, 0.3));
        });

        meshes.push_back(mesh);

        auto groundShape = std::make_shared<btBoxShape>(btVector3(50.f, 0.5, 50.f));
        auto rigidbody = RigidBody::create(node);
        rigidbody->setCollisionShape(groundShape);
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
            mesh->setShader(blinnPhongSolidColorShader);
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
        pointLight->ambient = glm::vec3(0.1);
        pointLight->diffuse = glm::vec3(1.0);
        pointLight->specular = glm::vec3(0.4);
        pointLight->distance = 10.0f;

        const auto directLight = DirectLight::create(node);
        directLight->ambient = glm::vec3(0.1);
        directLight->diffuse = glm::vec3(0.7);
        directLight->specular = glm::vec3(0.3);
    }

    void createCube() {
        const auto node = meshManager->createNodeFromMeshData("cube.obj", rootNode);
        node->name = "cube";
        node->transform()->setScale(1);

        auto mesh = node->getComponent<MeshComponent>();

        cubeDiffuseTexture = Engine::instance().textureManager()->getTexture("diffuse.png");
        cubeSpecularTexture = Engine::instance().textureManager()->getTexture("specular.png");

        mesh->setShader(blinnPhongShader);
        mesh->setBeforeDrawCallback([this](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setTexture("diffuseTexture", cubeDiffuseTexture);
            shaderProgram->setTexture("specularTexture", cubeSpecularTexture);
        });
        meshes.push_back(mesh);

        auto cubeShape = std::make_shared<btBoxShape>(btVector3(0.5f, 0.5f, 0.5f));
        auto rigidbody = RigidBody::create(node);
        rigidbody->setMass(1.f);
        rigidbody->setCollisionShape(cubeShape);
    }
};
