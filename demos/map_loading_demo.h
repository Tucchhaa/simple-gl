#pragma once

#include <fstream>
#include <sstream>

#include <unordered_map>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>

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
#include "../managers/physics_manager.h"
#include "../managers/window_manager.h"

using namespace SimpleGL;

class MapLoadingDemo {
    std::shared_ptr<Node> rootNode;
    std::shared_ptr<Node> staticNode;
    std::shared_ptr<MeshManager> meshManager;
    std::shared_ptr<btDynamicsWorld> dynamicsWorld;

    std::shared_ptr<ShaderProgram> blinnPhongShader;
    std::shared_ptr<ShaderProgram> skyboxShader;
    std::shared_ptr<ShaderProgram> solidColorShader;

    std::shared_ptr<Texture> skyboxTexture;

    Material2Tex m_defaultMaterial;
    std::unordered_map<std::string, Material2Tex> m_materialMap;

    std::shared_ptr<Camera> camera;

    std::vector<std::shared_ptr<MeshComponent>> meshes;
    std::shared_ptr<MeshComponent> skyboxCubeMesh;

public:
    std::shared_ptr<Scene> scene;

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

    void updateNodes() {
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

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        for (const auto& mesh : meshes) {
            mesh->draw(camera);
        }

        glCullFace(GL_FRONT);
        glDepthFunc(GL_LEQUAL);
        skyboxCubeMesh->draw(camera);
        glDepthFunc(GL_LESS);
        glCullFace(GL_BACK);
    }

private:
    void createScene() {
        createShaders();
        createPlayer();
        createSkybox();
        createLightSource();
        
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
    }

    void createPlayer() {
        auto playerNode = Node::create("playerNode", rootNode);
        playerNode->transform()->setPosition(0, 10, 10);

        auto playerShape = std::make_shared<btCapsuleShape>(0.5f, 1.5f);
        auto rigidBody = RigidBody::create(playerNode, "playerRigidBody");
        rigidBody->setMass(70.f);
        rigidBody->setCollisionShape(playerShape);
        rigidBody->init();

        auto cameraNode = Node::create("cameraNode", playerNode);
        cameraNode->transform()->setPosition(0, 0.8f, 0);
        camera = Camera::create(
            cameraNode,
            glm::radians(90.0f),
            0.3f,
            200.0f
        );

        auto controller = CharacterController::create(playerNode, "playerController");
        controller->setCameraNode(cameraNode);
        controller->setRigidBody(rigidBody);
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
            rigidBody->init();
            
            count++;
        }
        std::cout << "Loaded " << count << " objects with physics." << std::endl;
    }
};
