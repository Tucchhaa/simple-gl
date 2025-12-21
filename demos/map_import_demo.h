#pragma once

#include <iostream>
#include <filesystem>

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

class MapImportDemo {
    // Use SimpleGL::Material2Tex from MapLoader instead of defining our own
    std::shared_ptr<Node> rootNode;
    std::shared_ptr<Node> staticNode;

    std::shared_ptr<MeshManager> meshManager;
    std::shared_ptr<btDynamicsWorld> dynamicsWorld;

    std::shared_ptr<ShaderProgram> solidColorShader;
    std::shared_ptr<ShaderProgram> blinnPhongShader;
    std::shared_ptr<ShaderProgram> skyboxShader;

    std::shared_ptr<Texture> skyboxTexture;

    Material2Tex m_defaultMaterial;  // SimpleGL::Material2Tex

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
        
        // Physics simulation
        dynamicsWorld->stepSimulation(input->deltaTime());
    }

    void draw() {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // Clear color & depth buffers
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClearColor(0.f, 0.f, 0.f, 1.f);

        glDepthMask(GL_TRUE);
        glClearDepth(1);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw all meshes
        for (const auto& mesh : meshes) {
            mesh->draw(camera);
        }

        // Draw skybox (front face culling, depth func LEQUAL)
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

        skyboxShader = Engine::instance().shaderManager()->createShaderProgram(
            "shaders/skybox/vertex.glsl",
            "shaders/skybox/fragment.glsl",
            "Skybox Shader"
        );
    }

    void loadMaterials() {
        const auto tm = Engine::instance().textureManager();

        // Load default material (same for all objects)
        try {
            m_defaultMaterial.albedo = tm->getTexture("wall.jpg", true);
            m_defaultMaterial.specular = tm->getTexture("specular.png", false);
        } catch (...) {
            // Fallback to diffuse/specular if wall.jpg doesn't exist
            m_defaultMaterial.albedo = tm->getTexture("diffuse.png", true);
            m_defaultMaterial.specular = tm->getTexture("specular.png", false);
        }
    }

    void createPlayer() {
        auto playerNode = Node::create("playerNode", rootNode);
        playerNode->transform()->setPosition(0, 1, 5);

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
            200.0f
        );

        auto controller = CharacterController::create(playerNode, "playerController");
        controller->setCameraNode(cameraNode);
        controller->setRigidBody(rigidBody);
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

    void createLighting() {
        // Create a directional light for overall illumination
        auto directLightNode = Node::create("directLightNode", rootNode);
        
        // Position doesn't matter for directional light
        directLightNode->transform()->setPosition(0, 10, 0);
        
        // Rotate to point downward (-Y direction, typical for sun light)
        // Default forward is +Z, we want -Y, so rotate 90° around X axis
        directLightNode->transform()->setOrientation(
            glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))
        );
        
        const auto directLight = DirectLight::create(directLightNode);
        directLight->ambient = glm::vec3(0.15f, 0.15f, 0.2f);
        directLight->diffuse = glm::vec3(0.8f, 0.8f, 0.9f);
        directLight->specular = glm::vec3(0.3f, 0.3f, 0.3f);

        // Add a few point lights for better illumination
        const glm::vec3 lightPositions[] = {
            glm::vec3(5, 8, 5),
            glm::vec3(-5, 8, 5),
            glm::vec3(5, 8, -5),
            glm::vec3(-5, 8, -5),
        };

        for (size_t i = 0; i < 4; i++) {
            auto lightNode = Node::create("pointLight" + std::to_string(i), rootNode);
            lightNode->transform()->setPosition(lightPositions[i]);

            const auto pointLight = PointLight::create(lightNode);
            pointLight->ambient = glm::vec3(0.05f);
            pointLight->diffuse = glm::vec3(0.8f, 0.8f, 1.0f);
            pointLight->specular = glm::vec3(0.3f);
            pointLight->distance = 25.0f;
        }
    }

    void loadMap() {
        try {
            auto mapLoader = MapLoader::create();
            
            // Set default material for all objects
            mapLoader->setDefaultMaterial(m_defaultMaterial);
            
            // Set shader
            mapLoader->setBlinnPhongShader(blinnPhongShader);
            
            // Load map
            auto mapRoot = mapLoader->loadMap("maps/blender_export.json", staticNode);
            
            // Collect all mesh components from the loaded map for rendering
            auto loadedMeshes = mapRoot->getChildComponents<MeshComponent>();
            for (const auto& mesh : loadedMeshes) {
                meshes.push_back(mesh);
            }
            
            // Also check the root node itself for a mesh component
            if (auto rootMesh = mapRoot->getComponent<MeshComponent>()) {
                meshes.push_back(rootMesh);
            }
            
            std::cout << "MapImportDemo: Successfully loaded " << meshes.size() << " meshes from map" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "MapImportDemo: Failed to load map: " << e.what() << std::endl;
            std::cerr << "MapImportDemo: Running with empty scene (no map loaded)" << std::endl;
        }
    }
};

