#pragma once

#include <iostream>
#include <filesystem>
#include <unordered_map>

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
#include "../entities/components/controllers/free_controller.h"
#include "../entities/mesh_data.h"
#include "../managers/physics_manager.h"
#include "../managers/window_manager.h"

using namespace SimpleGL;

class MapImportDemo {
    std::shared_ptr<Node> rootNode;
    std::shared_ptr<Node> staticNode;

    std::shared_ptr<MeshManager> meshManager;
    std::shared_ptr<btDynamicsWorld> dynamicsWorld;

    std::shared_ptr<ShaderProgram> blinnPhongShader;
    std::shared_ptr<ShaderProgram> skyboxShader;

    std::shared_ptr<Texture> skyboxTexture;

    Material2Tex m_defaultMaterial;
    std::unordered_map<std::string, Material2Tex> m_materialMap;

    std::shared_ptr<Camera> camera;

    std::vector<std::shared_ptr<MeshComponent>> meshes;
    std::shared_ptr<MeshComponent> skyboxCubeMesh;

public:
    std::shared_ptr<Scene> scene;

    MapImportDemo() {
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
        const auto input = Engine::instance().windowManager()->mainWindow()->input();
        dynamicsWorld->stepSimulation(input->deltaTime());
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
        loadMaterials();
        createSkybox();
        createLighting();
        
        staticNode = Node::create("staticNode", rootNode);
        
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
    }

    void loadMaterials() {
        const auto tm = Engine::instance().textureManager();
        
        Material2Tex wallMaterial;
        try {
            wallMaterial.albedo = tm->getTexture("wall.jpg", true);
            wallMaterial.specular = tm->getTexture("specular.png", false);
            std::cout << "MapImportDemo: Loaded 'Wall' material textures." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "MapImportDemo: CRITICAL - Failed to load wall textures: " << e.what() << std::endl;
        }

        Material2Tex floorMaterial;
        try {
            floorMaterial.albedo = tm->getTexture("16942.jpg", true);
            floorMaterial.specular = tm->getTexture("specular.png", false);
            std::cout << "MapImportDemo: Loaded 'Floor' material textures." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "MapImportDemo: CRITICAL - Failed to load floor textures: " << e.what() << std::endl;
        }
        
        m_materialMap["Wall"] = wallMaterial;
        m_materialMap["Floor"] = floorMaterial;
        m_defaultMaterial = wallMaterial;
    }

    void createPlayer() {
        auto cameraNode = Node::create("cameraNode", rootNode);
        cameraNode->transform()->setPosition(0, 5, 15);
        
        camera = Camera::create(
            cameraNode,
            glm::radians(90.0f),
            0.3f,
            200.0f
        );

        FreeController::create(cameraNode);
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

    void createLighting() {
        auto directLightNode = Node::create("directLightNode", rootNode);
        directLightNode->transform()->setOrientation(
            glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))
        );
        
        const auto directLight = DirectLight::create(directLightNode);
        directLight->ambient = glm::vec3(0.15f, 0.15f, 0.2f);
        directLight->diffuse = glm::vec3(0.8f, 0.8f, 0.9f);
        directLight->specular = glm::vec3(0.3f, 0.3f, 0.3f);
    }

    void loadMap() {
        try {
            std::cout << "MapImportDemo: Starting map load..." << std::endl;
            
            auto mapLoader = MapLoader::create();
            
            for (const auto& pair : m_materialMap) {
                mapLoader->addMaterial(pair.first, pair.second);
            }
            mapLoader->setDefaultMaterial(m_defaultMaterial);
            mapLoader->setBlinnPhongShader(blinnPhongShader);
            
            auto mapRoot = mapLoader->loadMap("maps/blender_export.json", staticNode);
            
            auto loadedMeshes = mapRoot->getChildComponents<MeshComponent>();
            meshes.insert(meshes.end(), loadedMeshes.begin(), loadedMeshes.end());
            
            std::cout << "MapImportDemo: Successfully loaded " << loadedMeshes.size() << " meshes from map" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "MapImportDemo: Failed to load map: " << e.what() << std::endl;
        }
    }
};