#pragma once

#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>

#include "../managers/engine.h"
#include "../managers/mesh_manager.h"
#include "../managers/shader_manager.h"
#include "../managers/texture_manager.h"
#include "../window/window.h"
#include "../window/input.h"

#include "../entities/node.h"
#include "../entities/scene.h"
#include "../entities/shader_program.h"
#include "../entities/components/camera.h"
#include "../entities/components/light.h"
#include "../entities/components/mesh.h"
#include "../entities/components/rigid_body.h"
#include "../entities/components/transform.h"
#include "../entities/components/controllers/character_controller.h"
#include "../entities/components/controllers/free_controller.h"
#include "../entities/components/portal/portal_bullet.h"
#include "../entities/components/portal/portal_fps_controller.h"
#include "../entities/components/portal/teleportable.h"
#include "../managers/physics_manager.h"

#include "../render-pipeline/portal/portal.h"

using namespace SimpleGL;

enum CollisionGroups {
    GROUP_PLAYER = 1 << 0,
    GROUP_BULLET = 1 << 1,
    GROUP_ALLOW_PORTAL = 1 << 2,
    GROUP_OTHER = 1 << 3,
};

class BasicDemo {
    std::shared_ptr<Node> rootNode;
    
    static const std::unique_ptr<MeshManager>& meshManager() {
        return Engine::get()->meshManager();
    }
    
    static const std::unique_ptr<btDynamicsWorld>& dynamicsWorld() {
        return Engine::get()->physicsManager()->dynamicsWorld();
    }

    std::shared_ptr<ShaderProgram> basicShader;
    std::shared_ptr<ShaderProgram> solidColorShader;
    std::shared_ptr<ShaderProgram> blinnPhongShader;
    std::shared_ptr<ShaderProgram> shadedSolidColorShader;
    std::shared_ptr<ShaderProgram> skyboxShader;

    std::shared_ptr<Texture> skyboxTexture;
    std::shared_ptr<Texture> cubeDiffuseTexture;
    std::shared_ptr<Texture> cubeSpecularTexture;

    // This node contains static elements of the scene
    std::shared_ptr<Node> staticNode;

    std::vector<std::shared_ptr<MeshComponent>> meshes;
    std::vector<std::shared_ptr<Teleportable>> teleportables;
    std::shared_ptr<MeshComponent> skyboxCubeMesh;

public:
    std::shared_ptr<Portal> portal;

    std::shared_ptr<Camera> camera;
    std::shared_ptr<Scene> scene;

    BasicDemo() {
        scene = std::make_shared<Scene>();
        Engine::get()->setScene(scene);

        rootNode = Node::create("ROOT");
        scene->setRootNode(rootNode);

        dynamicsWorld()->setGravity(btVector3(0, -10.f, 0));

        createScene();
    }

    void stepPhysicsSimulation() {
        float timeStep = Engine::get()->window()->input()->deltaTime();
        dynamicsWorld()->stepSimulation(timeStep);
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

            for (const auto& teleportable : teleportables) {
                teleportable->draw(_camera);
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

        // portal->applyCameraNearPlane();

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
        createShaders();

        createCamera();

        createSkybox();
        createPortal();

        staticNode = Node::create("staticNode", rootNode);
        createGround();
        createWalls();

        createLightSource();
        createCube();

        // Uncomment one of these:
        // createFreeCameraController();
        createFPSController();
    }

    void createShaders() {
        basicShader = Engine::get()->shaderManager()->createShaderProgram(
            "shaders/basic/vertex.glsl",
            "shaders/basic/fragment.glsl",
            "basic shader program"
        );

        solidColorShader = Engine::get()->shaderManager()->createShaderProgram(
            "shaders/solid-color/vertex.glsl",
            "shaders/solid-color/fragment.glsl",
            "solid color shader program"
        );

        blinnPhongShader = Engine::get()->shaderManager()->createShaderProgram(
            "shaders/blinn-phong/vertex.glsl",
            "shaders/blinn-phong/fragment.glsl",
            "Blinn-Phong Shader"
        );

        shadedSolidColorShader = Engine::get()->shaderManager()->createShaderProgram(
            "shaders/shaded-solid-color/vertex.glsl",
            "shaders/shaded-solid-color/fragment.glsl",
            "Blinn-Phong Solid Color Shader"
        );

        skyboxShader = Engine::get()->shaderManager()->createShaderProgram(
            "shaders/skybox/vertex.glsl",
            "shaders/skybox/fragment.glsl",
            "Skybox Shader"
        );
    }

    void createCamera() {
        auto cameraNode = Node::create("cameraNode", rootNode);
        cameraNode->transform()->setPosition(0, 0.7, 0);

        camera = Camera::Factory::create(
            cameraNode,
            glm::radians(90.0f),
            0.1f,
            100.0f
        );
    }

    void createFPSController() {
        auto playerNode = Node::create("playerNode", rootNode);
        playerNode->transform()->setPosition(0, 0, 3);

        auto meshNode = Node::create("playerMeshNode", playerNode);
        meshNode->transform()->setScale(0.3f);

        auto capsule = meshManager()->loadMeshData("./capsule.obj");
        auto playerMesh = MeshComponent::Factory::create(meshNode, capsule, "Player Mesh");
        playerMesh->setShader(shadedSolidColorShader);
        playerMesh->setBeforeDrawCallback([](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setUniform("color", glm::vec3(0.6, 0.6, 0.6));
        });

        meshes.push_back(playerMesh);

        auto playerShape = std::make_shared<btCapsuleShape>(0.5f, 0.8f);
        auto rigidBody = RigidBody::Factory::create(playerNode, "playerRigidBody");
        rigidBody->setMass(70.f);
        rigidBody->setCollisionShape(playerShape);
        rigidBody->group = GROUP_PLAYER;
        rigidBody->mask &= ~GROUP_BULLET;
        rigidBody->init();

        // Camera
        camera->node()->setParent(playerNode);
        camera->node()->transform()->setPosition(0, 0.5, 0);

        // Weapon
        auto weaponPivotNode = Node::create("weaponPivotNode", camera->node());
        weaponPivotNode->transform()->setPosition(0, 0, -0.6);

        auto weaponNode = Node::create("weaponNode", weaponPivotNode);
        weaponNode->transform()->setScale(0.2, 0.2, 0.9);
        weaponNode->transform()->setPosition(0.5, -0.35, 0);

        auto cube = meshManager()->loadMeshData("./cube.obj");
        auto weaponMesh = MeshComponent::Factory::create(weaponNode, cube, "Weapon Mesh");
        weaponMesh->setShader(shadedSolidColorShader);
        weaponMesh->setBeforeDrawCallback([](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setUniform("color", glm::vec3(0.8, 0.5, 0.5));
        });

        meshes.push_back(weaponMesh);

        // Bullets
        auto sphere = meshManager()->loadMeshData("./sphere.obj");
        const auto bulletShape = std::make_shared<btSphereShape>(0.1f);

        auto createBullet = [&](const std::string& name, const std::shared_ptr<Node>& portalNode) {
            auto bulletNode = Node::create(name, rootNode);
            bulletNode->transform()->setScale(0.1);

            auto bulletMesh = MeshComponent::Factory::create(bulletNode, sphere, "Bullet Mesh");
            bulletMesh->setShader(solidColorShader);

            meshes.push_back(bulletMesh);

            const auto bulletRigidBody = RigidBody::Factory::create(bulletNode, "bulletRigidBody");
            bulletRigidBody->setMass(1.f);
            bulletRigidBody->setCollisionShape(bulletShape);
            bulletRigidBody->group = GROUP_BULLET;
            bulletRigidBody->mask = GROUP_ALLOW_PORTAL;
            bulletRigidBody->init();

            auto bullet = PortalBullet::Factory::create(bulletNode);
            bullet->setPortalNode(portalNode);
            bullet->setRigidBody(bulletRigidBody);

            return bulletNode;
        };

        auto bullet1Node = createBullet("bullet1Node", portal->portal1Node);
        bullet1Node->getComponent<MeshComponent>()->setBeforeDrawCallback([](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setUniform("color", glm::vec3(0.1, 0.1, 0.8));
        });

        auto bullet2Node = createBullet("bullet2Node", portal->portal2Node);
        bullet2Node->getComponent<MeshComponent>()->setBeforeDrawCallback([](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setUniform("color", glm::vec3(0.8, 0.1, 0.1));
        });

        // Controller
        auto controller = PortalFPSController::Factory::create(playerNode, "playerController");
        controller->setCameraNode(camera->node());
        controller->setRigidBody(rigidBody);
        controller->setWeaponNode(weaponNode);
        controller->setPortal1Bullet(bullet1Node->getComponent<PortalBullet>());
        controller->setPortal2Bullet(bullet2Node->getComponent<PortalBullet>());

        // Teleportable
        auto teleportable = Teleportable::Factory::create(playerNode, "playerTeleportable");
        teleportable->setPortal(portal);
        teleportable->setAllowPortalGroup(GROUP_ALLOW_PORTAL);

        teleportable->setMeshes({ playerMesh, weaponMesh });

        teleportables.push_back(teleportable);
    }

    void createFreeCameraController() {
        FreeController::Factory::create(camera->node());
    }

    void createSkybox() {
        skyboxTexture = Engine::get()->textureManager()->getCubeMapTexture(
            "sky",
            "skybox/right.jpg",
            "skybox/left.jpg",
            "skybox/top.jpg",
            "skybox/bottom.jpg",
            "skybox/front.jpg",
            "skybox/back.jpg",
            false
        );

        auto node = meshManager()->createNodeFromMeshData("cube.obj", rootNode);
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

        const auto planeMesh = meshManager()->loadMeshData("plane.obj");
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
        const auto node = meshManager()->createNodeFromMeshData("cube.obj", staticNode);
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
        auto rigidbody = RigidBody::Factory::create(node);
        rigidbody->setCollisionShape(groundShape);
        rigidbody->group = GROUP_OTHER;
        rigidbody->mask &= ~GROUP_BULLET;
        rigidbody->init();
    }

    void createWalls() {
        glm::vec3 scale = glm::vec3(30, 4, 1);
        float positionX[] = { 0.0f, 0.5f, 0.0f, -0.5f, -0.3f };
        float positionZ[] = { 0.5f, 0.0f, -0.5f, 0.0f, 0.3f, 0.5f };
        bool rotate[] = { false, true, false, true, true };

        auto wallShape = std::make_shared<btBoxShape>(btVector3(scale.x / 2.f, scale.y / 2.f, scale.z / 2.f));

        for (int i=0; i < 5; i++) {
            auto node = meshManager()->createNodeFromMeshData("cube.obj", staticNode);
            node->name = "wallNode"+std::to_string(i);
            node->transform()->setScale(scale.x, scale.y, scale.z);
            node->transform()->setPosition(
                positionX[i] * scale.x,
                -3.0f,
                positionZ[i] * scale.x
            );

            if (rotate[i]) {
                auto r = glm::angleAxis(glm::radians(90.f), glm::vec3(0, 1, 0));
                node->transform()->rotate(r);
            }

            auto mesh = node->getComponent<MeshComponent>();
            mesh->setShader(shadedSolidColorShader);
            mesh->setBeforeDrawCallback([](const std::shared_ptr<ShaderProgram>& shaderProgram) {
                shaderProgram->setUniform("color", glm::vec3(0.3, 0.3, 0.3));
            });

            meshes.push_back(mesh);

            auto rigidbody = RigidBody::Factory::create(node);
            rigidbody->setCollisionShape(wallShape);
            rigidbody->group = GROUP_ALLOW_PORTAL;
            rigidbody->init();
        }
    }

    void createLightSource() {
        const auto node = meshManager()->createNodeFromMeshData("cube.obj", rootNode);
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
        const auto pointLight = PointLight::Factory::create(node);
        pointLight->ambient = glm::vec3(0.0);
        pointLight->diffuse = glm::vec3(0.7);
        pointLight->specular = glm::vec3(0.2);
        pointLight->distance = 10.0f;

        const auto directLight = DirectLight::Factory::create(node);
        directLight->ambient = glm::vec3(0.05);
        directLight->diffuse = glm::vec3(0.25);
        directLight->specular = glm::vec3(0.04);
    }

    void createCube() {
        const auto node = meshManager()->createNodeFromMeshData("cube.obj", rootNode);
        node->name = "cube";
        node->transform()->setScale(1);

        auto mesh = node->getComponent<MeshComponent>();

        cubeDiffuseTexture = Engine::get()->textureManager()->getTexture("diffuse.png", true);
        cubeSpecularTexture = Engine::get()->textureManager()->getTexture("specular.png", false);

        mesh->setShader(blinnPhongShader);
        mesh->setBeforeDrawCallback([this](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setTexture("diffuseTexture", cubeDiffuseTexture);
            shaderProgram->setTexture("specularTexture", cubeSpecularTexture);
        });
        meshes.push_back(mesh);

        auto cubeShape = std::make_shared<btBoxShape>(btVector3(0.5f, 0.5f, 0.5f));
        auto rigidBody = RigidBody::Factory::create(node);
        rigidBody->setMass(40.f);
        rigidBody->setCollisionShape(cubeShape);
        rigidBody->group = GROUP_OTHER;
        rigidBody->mask &= ~GROUP_BULLET;
        rigidBody->init();

        // Teleportable
        auto teleportable = Teleportable::Factory::create(node, "playerTeleportable");
        teleportable->setPortal(portal);
        teleportable->setAllowPortalGroup(GROUP_ALLOW_PORTAL);
        teleportable->setMeshes({ mesh });

        teleportables.push_back(teleportable);
    }
};
