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
#include "../entities/components/controllers/free_controller.h"
#include "../entities/mesh_data.h"
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

        // Enable depth testing (CRITICAL - was missing!)
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // Draw all meshes
        static int drawCallCount = 0;
        drawCallCount++;
        if (drawCallCount <= 5) {  // Log first 5 draw calls
            std::cout << "MapImportDemo::draw() - Drawing " << meshes.size() << " meshes (draw call #" << drawCallCount << ")" << std::endl;
            if (!meshes.empty()) {
                std::cout << "  First mesh node name: " << meshes[0]->node()->name << std::endl;
                std::cout << "  First mesh visible: " << meshes[0]->node()->visible << std::endl;
            }
        }
        
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
        
        // Create test square first
        createTestSquare();
        
        // Then load map
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
            std::cout << "MapImportDemo: Loaded textures - wall.jpg and specular.png" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "MapImportDemo: Failed to load wall.jpg, trying fallback: " << e.what() << std::endl;
            // Fallback to diffuse/specular if wall.jpg doesn't exist
            try {
                m_defaultMaterial.albedo = tm->getTexture("diffuse.png", true);
                m_defaultMaterial.specular = tm->getTexture("specular.png", false);
                std::cout << "MapImportDemo: Loaded fallback textures - diffuse.png and specular.png" << std::endl;
            } catch (const std::exception& e2) {
                std::cerr << "MapImportDemo: CRITICAL - Failed to load ANY textures: " << e2.what() << std::endl;
                std::cerr << "MapImportDemo: Objects will not render without textures!" << std::endl;
            }
        }
        
        std::cout << "MapImportDemo: Material status - albedo: " << (m_defaultMaterial.albedo != nullptr) 
                  << ", specular: " << (m_defaultMaterial.specular != nullptr) << std::endl;
    }

    void createPlayer() {
        // Use free camera (flying mode) instead of character controller
        auto cameraNode = Node::create("cameraNode", rootNode);
        // Position camera to see the test square and map objects
        cameraNode->transform()->setPosition(0, 0, 3);
        
        std::cout << "MapImportDemo: Free camera created at position (0, 0, 3)" << std::endl;

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

    void createTestSquare() {
        // Create a simple square at (0, 0, 0) for testing
        // Square facing forward (+Z), 1x1 unit size
        // Vertex format: position(3) + UV(2) + normal(3) = 8 floats per vertex
        
        std::vector<float> vertices = {
            // Bottom-left
            -0.5f, -0.5f, 0.0f,  // position
            0.0f, 0.0f,          // UV
            0.0f, 0.0f, 1.0f,    // normal (forward)
            
            // Bottom-right
            0.5f, -0.5f, 0.0f,
            1.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
            
            // Top-right
            0.5f, 0.5f, 0.0f,
            1.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            
            // Top-left
            -0.5f, 0.5f, 0.0f,
            0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
        };
        
        std::vector<unsigned int> indices = {
            0, 1, 2,  // First triangle
            0, 2, 3   // Second triangle
        };
        
        auto testMeshData = MeshData::createFromArrays("testSquare", vertices, indices, 8);
        
        auto testNode = Node::create("testSquareNode", staticNode);
        testNode->transform()->setPosition(0, 0, 0);
        
        auto testMeshComponent = MeshComponent::create(testNode, testMeshData, "testSquareMesh");
        testMeshComponent->setShader(blinnPhongShader);
        
        // Apply default material
        testMeshComponent->setBeforeDrawCallback([this](const std::shared_ptr<ShaderProgram>& shader) {
            if (m_defaultMaterial.albedo && m_defaultMaterial.specular) {
                shader->setTexture("diffuseTexture", m_defaultMaterial.albedo);
                shader->setTexture("specularTexture", m_defaultMaterial.specular);
            }
        });
        
        meshes.push_back(testMeshComponent);
        std::cout << "MapImportDemo: Created test square at (0, 0, 0)" << std::endl;
    }

    void loadMap() {
        try {
            std::cout << "MapImportDemo: Starting map load..." << std::endl;
            std::cout << "MapImportDemo: Default material - has albedo: " << (m_defaultMaterial.albedo != nullptr) 
                      << ", has specular: " << (m_defaultMaterial.specular != nullptr) << std::endl;
            std::cout << "MapImportDemo: BlinnPhong shader: " << (blinnPhongShader != nullptr) << std::endl;
            
            auto mapLoader = MapLoader::create();
            
            // Set default material for all objects
            mapLoader->setDefaultMaterial(m_defaultMaterial);
            
            // Set shader
            mapLoader->setBlinnPhongShader(blinnPhongShader);
            
            // Load map
            auto mapRoot = mapLoader->loadMap("maps/blender_export.json", staticNode);
            
            std::cout << "MapImportDemo: Map root created, collecting meshes..." << std::endl;
            
            // Collect all mesh components from the loaded map for rendering
            auto loadedMeshes = mapRoot->getChildComponents<MeshComponent>();
            std::cout << "MapImportDemo: Found " << loadedMeshes.size() << " child mesh components" << std::endl;
            
            for (const auto& mesh : loadedMeshes) {
                meshes.push_back(mesh);
            }
            
            // Also check the root node itself for a mesh component
            if (auto rootMesh = mapRoot->getComponent<MeshComponent>()) {
                std::cout << "MapImportDemo: Found mesh component on root node" << std::endl;
                meshes.push_back(rootMesh);
            }
            
            std::cout << "MapImportDemo: Successfully loaded " << meshes.size() << " meshes from map" << std::endl;
            std::cout << "MapImportDemo: Total meshes in scene: " << meshes.size() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "MapImportDemo: Failed to load map: " << e.what() << std::endl;
            std::cerr << "MapImportDemo: Running with empty scene (no map loaded)" << std::endl;
        }
    }
};

