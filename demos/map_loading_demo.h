#pragma once

#include <fstream>
#include <sstream>

#include <unordered_map>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>

#include "../entities/input.h"
#include "../managers/engine.h"
#include "../managers/mesh_manager.h"
#include "../managers/shader_manager.h"
#include "../managers/texture_manager.h"
#include "../managers/map_loader.h"

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
#include "../entities/components/portal/portal_bullet.h"
#include "../entities/components/portal/portal_fps_controller.h"
#include "../entities/components/portal/teleportable.h"
#include "../managers/physics_manager.h"
#include "../managers/window_manager.h"
#include "../render-pipeline/portal/portal.h"

using namespace SimpleGL;

enum CollisionGroups {
    GROUP_PLAYER = 1 << 0,
    GROUP_BULLET = 1 << 1,
    GROUP_ALLOW_PORTAL = 1 << 2,
    GROUP_OTHER = 1 << 3,
};

class MapLoadingDemo {
    std::shared_ptr<Node> rootNode;
    std::shared_ptr<Node> staticNode;
    std::shared_ptr<MeshManager> meshManager;
    std::shared_ptr<btDynamicsWorld> dynamicsWorld;

    std::shared_ptr<ShaderProgram> blinnPhongShader;
    std::shared_ptr<ShaderProgram> skyboxShader;
    std::shared_ptr<ShaderProgram> solidColorShader;
    std::shared_ptr<ShaderProgram> shadedSolidColorShader;

    std::shared_ptr<Texture> skyboxTexture;
    std::shared_ptr<Texture> cubeDiffuseTexture;
    std::shared_ptr<Texture> cubeSpecularTexture;

    Material2Tex m_defaultMaterial;
    std::unordered_map<std::string, Material2Tex> m_materialMap;

    std::shared_ptr<Portal> portal;
    std::vector<std::shared_ptr<Teleportable>> teleportables;
    
    // Store player references for teleportable setup
    std::shared_ptr<Node> playerNode;
    std::shared_ptr<RigidBody> playerRigidBody;

    std::vector<std::shared_ptr<MeshComponent>> meshes;
    std::shared_ptr<MeshComponent> skyboxCubeMesh;

public:
    std::shared_ptr<Scene> scene;
    std::shared_ptr<Camera> camera;

    MapLoadingDemo() {
        scene = Scene::create();
        Engine::instance().setScene(scene);

        rootNode = Node::create("ROOT");
        scene->setRootNode(rootNode);

        meshManager = Engine::instance().meshManager();
        dynamicsWorld = Engine::instance().physicsManager()->dynamicsWorld();
        dynamicsWorld->setGravity(btVector3(0, -10.f, 0));

        createScene();
    }

    void stepPhysicsSimulation() {
        float timeStep = Engine::instance().windowManager()->mainWindow()->input()->deltaTime();
        dynamicsWorld->stepSimulation(timeStep);
    }

    void draw() {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClearColor(0.f, 0.f, 0.f, 1.f);
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
        createLightSource();
        createPortal();
        createFPSController();
        createCube();
        
        staticNode = Node::create("staticNode", rootNode);
        
        loadMaterials();
        loadMap();
    }

    void createShaders() {
        blinnPhongShader = Engine::instance().shaderManager()->createShaderProgram(
            "shaders/blinn-phong/vertex.glsl",
            "shaders/blinn-phong/fragment.glsl",
            "Blinn-Phong Shader"
        );

        skyboxShader = Engine::instance().shaderManager()->createShaderProgram(
            "shaders/skybox/vertex.glsl",
            "shaders/skybox/fragment.glsl",
            "Skybox Shader"
        );

        solidColorShader = Engine::instance().shaderManager()->createShaderProgram(
            "shaders/solid-color/vertex.glsl",
            "shaders/solid-color/fragment.glsl",
            "solid color shader program"
        );

        shadedSolidColorShader = Engine::instance().shaderManager()->createShaderProgram(
            "shaders/shaded-solid-color/vertex.glsl",
            "shaders/shaded-solid-color/fragment.glsl",
            "Blinn-Phong Solid Color Shader"
        );
    }

    void createCamera() {
        auto cameraNode = Node::create("cameraNode", rootNode);
        cameraNode->transform()->setPosition(0, 0.8f, 0);
        camera = Camera::create(
            cameraNode,
            glm::radians(90.0f),
            0.3f,
            200.0f
        );
    }

    void createFPSController() {
        playerNode = Node::create("playerNode", rootNode);
        playerNode->transform()->setPosition(0, 10, 10);

        auto meshNode = Node::create("playerMeshNode", playerNode);
        meshNode->transform()->setScale(0.3f);

        auto capsule = meshManager->loadMeshData("./capsule.obj");
        auto playerMesh = MeshComponent::create(meshNode, capsule, "Player Mesh");
        playerMesh->setShader(shadedSolidColorShader);
        playerMesh->setBeforeDrawCallback([](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setUniform("color", glm::vec3(0.6, 0.6, 0.6));
        });

        meshes.push_back(playerMesh);

        auto playerShape = std::make_shared<btCapsuleShape>(0.5f, 0.8f);
        playerRigidBody = RigidBody::create(playerNode, "playerRigidBody");
        playerRigidBody->setMass(70.f);
        playerRigidBody->setCollisionShape(playerShape);
        playerRigidBody->group = GROUP_PLAYER;
        playerRigidBody->mask &= ~GROUP_BULLET;
        playerRigidBody->init();

        // Camera
        camera->node()->setParent(playerNode);
        camera->node()->transform()->setPosition(0, 0.5, -0.1);

        // Weapon
        auto weaponPivotNode = Node::create("weaponPivotNode", camera->node());
        weaponPivotNode->transform()->setPosition(0, 0, -0.6);

        auto weaponNode = Node::create("weaponNode", weaponPivotNode);
        weaponNode->transform()->setScale(0.2, 0.2, 0.9);
        weaponNode->transform()->setPosition(0.5, -0.35, 0);

        auto cube = meshManager->loadMeshData("./cube.obj");
        auto weaponMesh = MeshComponent::create(weaponNode, cube, "Weapon Mesh");
        weaponMesh->setShader(shadedSolidColorShader);
        weaponMesh->setBeforeDrawCallback([](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setUniform("color", glm::vec3(0.8, 0.5, 0.5));
        });

        meshes.push_back(weaponMesh);

        // Bullets
        auto sphere = meshManager->loadMeshData("./sphere.obj");
        const auto bulletShape = std::make_shared<btSphereShape>(0.1f);

        auto createBullet = [&](const std::string& name, const std::shared_ptr<Node>& portalNode) {
            auto bulletNode = Node::create(name, rootNode);
            bulletNode->transform()->setScale(0.1);

            auto bulletMesh = MeshComponent::create(bulletNode, sphere, "Bullet Mesh");
            bulletMesh->setShader(solidColorShader);

            meshes.push_back(bulletMesh);

            const auto bulletRigidBody = RigidBody::create(bulletNode, "bulletRigidBody");
            bulletRigidBody->setMass(1.f);
            bulletRigidBody->setCollisionShape(bulletShape);
            bulletRigidBody->group = GROUP_BULLET;
            bulletRigidBody->mask = GROUP_ALLOW_PORTAL;
            bulletRigidBody->init();

            auto bullet = PortalBullet::create(bulletNode);
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
        auto controller = PortalFPSController::create(playerNode, "playerController");
        controller->setCameraNode(camera->node());
        controller->setRigidBody(playerRigidBody);
        controller->setWeaponNode(weaponNode);
        controller->setPortal1Bullet(bullet1Node->getComponent<PortalBullet>());
        controller->setPortal2Bullet(bullet2Node->getComponent<PortalBullet>());

        // Teleportable
        auto teleportable = Teleportable::create(playerNode, "playerTeleportable");
        teleportable->setPortal(portal);
        teleportable->setRigidBody(playerRigidBody);
        teleportable->setAllowPortalGroup(GROUP_ALLOW_PORTAL);
        teleportable->setMeshes({ playerMesh, weaponMesh });

        teleportables.push_back(teleportable);
    }

    void createSkybox() {
        skyboxTexture = Engine::instance().textureManager()->getCubeMapTexture(
            "sky",
            "skybox/right.jpg", "skybox/left.jpg", "skybox/top.jpg",
            "skybox/bottom.jpg", "skybox/front.jpg", "skybox/back.jpg",
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

    void createLightSource() {
        const auto node = meshManager->createNodeFromMeshData("cube.obj", rootNode);
        node->name = "lightSourceCube";
        node->transform()->setScale(0.2f);
        node->transform()->setPosition(0, 15, 0);

        auto mesh = node->getComponent<MeshComponent>();
        mesh->setShader(solidColorShader);
        mesh->setBeforeDrawCallback([](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setUniform("color", 1, 1, 1);
        });
        meshes.push_back(mesh);

        const auto directLight = DirectLight::create(node);
        directLight->ambient = glm::vec3(0.15f, 0.15f, 0.2f);
        directLight->diffuse = glm::vec3(0.8f, 0.8f, 0.9f);
        directLight->specular = glm::vec3(0.3f, 0.3f, 0.3f);
    }

    void createPortal() {
        auto node = Node::create("portal", rootNode);

        portal = Portal::create(camera);

        // position portals
        portal->portal1Node->transform()->setPosition(-8, -3, -14.45);
        portal->portal2Node->transform()->setPosition(-8, -3, -10);
        portal->portal2Node->transform()->rotate(glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 1.0f, 0.0f)));
        portal->portal2Node->transform()->rotate(glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 1.0f, 0.0f)));

        const auto planeMesh = meshManager->loadMeshData("plane.obj");
        portal->setPortalMesh(1, planeMesh);
        portal->setPortalMesh(2, planeMesh);

        glm::quat orientation = glm::quat(1, 0, 0, 0);
        orientation = glm::angleAxis(glm::radians(90.f), glm::vec3(1.0f, 0, 0)) * orientation;
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

    void createCube() {
        const auto node = meshManager->createNodeFromMeshData("cube.obj", rootNode);
        node->name = "cube";
        node->transform()->setScale(1);
        node->transform()->setPosition(0, 15, 0);

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
        rigidBody->setMass(40.f);
        rigidBody->setCollisionShape(cubeShape);
        rigidBody->group = GROUP_OTHER;
        rigidBody->mask &= ~GROUP_BULLET;
        rigidBody->init();

        // Teleportable
        auto teleportable = Teleportable::create(node, "playerTeleportable");
        teleportable->setPortal(portal);
        teleportable->setRigidBody(rigidBody);
        teleportable->setAllowPortalGroup(GROUP_ALLOW_PORTAL);
        teleportable->setMeshes({ mesh });

        teleportables.push_back(teleportable);
    }

    void loadMaterials() {
        const auto tm = Engine::instance().textureManager();
        
        Material2Tex wallMaterial;
        try {
            wallMaterial.albedo = tm->getTexture("Pav_DIFF.jpg", true);
            wallMaterial.specular = tm->getTexture("Pav_SPEC.jpg", false);
        } catch (const std::exception& e) {
            std::cerr << "MapLoadingDemo: CRITICAL - Failed to load wall textures: " << e.what() << std::endl;
        }

        Material2Tex floorMaterial;
        try {
            floorMaterial.albedo = tm->getTexture("Pav_DIFF.jpg", true);
            floorMaterial.specular = tm->getTexture("Pav_SPEC.jpg", false);
        } catch (const std::exception& e) {
            std::cerr << "MapLoadingDemo: CRITICAL - Failed to load floor textures: " << e.what() << std::endl;
        }
        
        m_materialMap["Wall"] = wallMaterial;
        m_materialMap["Floor"] = floorMaterial;
        m_defaultMaterial = wallMaterial;
    }

    // void loadMap() {
    //     try {
    //         auto mapLoader = MapLoader::create();
            
    //         for (const auto& pair : m_materialMap) {
    //             mapLoader->addMaterial(pair.first, pair.second);
    //         }
    //         mapLoader->setDefaultMaterial(m_defaultMaterial);
    //         mapLoader->setBlinnPhongShader(blinnPhongShader);
            
    //         auto mapRoot = mapLoader->loadMap("maps/blender_export.json", staticNode);
            
    //         auto loadedMeshes = mapRoot->getChildComponents<MeshComponent>();
    //         meshes.insert(meshes.end(), loadedMeshes.begin(), loadedMeshes.end());
            
    //         std::cout << "MapLoadingDemo: Successfully loaded " << loadedMeshes.size() << " meshes from map" << std::endl;
    //     } catch (const std::exception& e) {
    //         std::cerr << "MapLoadingDemo: Failed to load map: " << e.what() << std::endl;
    //     }
    // }
    void loadMap() {
        std::string path = "resources/maps/level.txt"; 
        std::ifstream f(path);
        
        if (!f.is_open()) {
            std::cerr << "CRITICAL: Could not open map file: " << path << std::endl;
            f.open("../resources/maps/level.txt");
            if(!f.is_open()) return;
        }

        std::cout << "Loading map from: " << path << std::endl;

        std::string name, matName;
        float px, py, pz;
        float rw, rx, ry, rz;
        float sx, sy, sz; // These are the full dimensions (width, height, depth)

        int count = 0;
        while (f >> name >> matName >> px >> py >> pz >> rw >> rx >> ry >> rz >> sx >> sy >> sz) {
            
            // --- 1. VISUALS ---
            auto node = meshManager->createNodeFromMeshData("cube.obj", staticNode);
            node->name = name;
            node->transform()->setPosition(px, py, pz);
            node->transform()->setOrientation(glm::quat(rw, rx, ry, rz));
            
            glm::vec3 scaleVec(sx, sy, sz);
            node->transform()->setScale(scaleVec);

            auto mesh = node->getComponent<MeshComponent>();
            mesh->setShader(blinnPhongShader);

            // Resolve Material
            Material2Tex matData = m_defaultMaterial;
            if (m_materialMap.find(matName) != m_materialMap.end()) {
                matData = m_materialMap[matName];
            } else {
                std::string baseName = matName.substr(0, matName.find('.'));
                if (m_materialMap.find(baseName) != m_materialMap.end()) {
                    matData = m_materialMap[baseName];
                }
            }

            // Texture Callback (With Tiling Fix)
            mesh->setBeforeDrawCallback([matData, scaleVec](const std::shared_ptr<ShaderProgram>& shader) {
                shader->setTexture("diffuseTexture", matData.albedo);
                shader->setTexture("specularTexture", matData.specular);
                if (shader->uniformExists("uvScale")) {
                    shader->setUniform("uvScale", scaleVec);
                }
            });

            meshes.push_back(mesh);

            // --- 2. PHYSICS (The Missing Part!) ---
            // Bullet Physics uses "Half-Extents" (distance from center to edge).
            // Since 'sx, sy, sz' are the full dimensions, we divide by 2.
            auto shape = std::make_shared<btBoxShape>(btVector3(sx / 2.0f, sy / 2.0f, sz / 2.0f));
            
            auto rigidBody = RigidBody::create(node);
            rigidBody->setCollisionShape(shape);
            rigidBody->setMass(0.0f); // 0.0f Mass = Static Object (Wall/Floor)
            rigidBody->group = GROUP_ALLOW_PORTAL; // Allow portals to be placed on map geometry
            rigidBody->init();
            
            count++;
        }
        std::cout << "Loaded " << count << " objects with physics." << std::endl;
    }
};
