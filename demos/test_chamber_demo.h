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

#include "../helpers/node_logger.h"
#include "../render-pipeline/portal/portal.h"

using namespace SimpleGL;

class TestChamberDemo {
    std::shared_ptr<Node> rootNode;
    std::shared_ptr<Node> staticNode;

    std::shared_ptr<MeshManager> meshManager;
    std::shared_ptr<btDynamicsWorld> dynamicsWorld;

    std::shared_ptr<ShaderProgram> solidColorShader;
    std::shared_ptr<ShaderProgram> shadedSolidColorShader;
    std::shared_ptr<ShaderProgram> blinnPhongShader;
    std::shared_ptr<ShaderProgram> skyboxShader;

    std::shared_ptr<Texture> skyboxTexture;

    // Placeholder materials (swap later for real kit textures)
    std::shared_ptr<Texture> wallDiffuseTexture;
    std::shared_ptr<Texture> wallSpecularTexture;
    std::shared_ptr<Texture> floorDiffuseTexture;
    std::shared_ptr<Texture> floorSpecularTexture;

    std::shared_ptr<Portal> portal;
    std::shared_ptr<Camera> camera;

    std::vector<std::shared_ptr<MeshComponent>> meshes;
    std::shared_ptr<MeshComponent> skyboxCubeMesh;

public:
    std::shared_ptr<Scene> scene;

    TestChamberDemo() {
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

        // physics
        dynamicsWorld->stepSimulation(input->deltaTime());

        // debug: dump node tree (names + components) to a file
        if (input->isKeyPressed(GLFW_KEY_F1)) {
            Logger logger("node_dump.txt");
            logger.logNode(rootNode);
        }
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

        const auto drawCall = [this](const std::shared_ptr<Camera>& _camera) {
            for (const auto& mesh : meshes) {
                mesh->draw(_camera);
            }

            // skybox
            glCullFace(GL_FRONT);
            glDepthFunc(GL_LEQUAL);
            skyboxCubeMesh->draw(_camera);
            glDepthFunc(GL_LESS);
            glCullFace(GL_BACK);
        };

        // draw portal contents
        portal->drawPortal(2, drawCall);
        portal->drawPortal(1, drawCall);

        // draw rest of scene
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        glDisable(GL_STENCIL_TEST);
        glStencilMask(0x00);

        drawCall(camera);
    }

private:
    // --------- Helpers (module spawner layer) ---------

    std::shared_ptr<Node> spawnVisualCube(
        const std::string& name,
        const glm::vec3& position,
        const glm::vec3& scale,
        const glm::quat& orientation,
        const std::shared_ptr<Node>& parent
    ) {
        auto node = meshManager->createNodeFromMeshData("cube.obj", parent);
        node->name = name;
        node->transform()->setPosition(position);
        node->transform()->setScale(scale);
        node->transform()->setOrientation(orientation);
        return node;
    }

    void applyTexturedMaterialToSubtree(
        const std::shared_ptr<Node>& node,
        const std::shared_ptr<Texture>& diffuse,
        const std::shared_ptr<Texture>& specular
    ) {
        auto subtreeMeshes = node->getChildComponents<MeshComponent>();

        // If the root itself has a mesh (common for cube.obj), include it too.
        if (auto rootMesh = node->getComponent<MeshComponent>()) {
            subtreeMeshes.push_back(rootMesh);
        }

        for (auto& m : subtreeMeshes) {
            m->setShader(blinnPhongShader);
            m->setBeforeDrawCallback([diffuse, specular](const std::shared_ptr<ShaderProgram>& s) {
                s->setTexture("diffuseTexture", diffuse);
                s->setTexture("specularTexture", specular);
            });
            meshes.push_back(m);
        }
    }

    void spawnStaticBoxCollider(
        const std::shared_ptr<Node>& node,
        const glm::vec3& halfExtents
    ) {
        auto shape = std::make_shared<btBoxShape>(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
        auto rb = RigidBody::create(node);
        rb->setCollisionShape(shape);
        rb->init();
    }

    // --------- Scene setup ---------

    void createScene() {
        createShaders();

        // Player camera/controller
        createPlayer();

        // Materials/textures (placeholders)
        createPlaceholderMaterials();

        createSkybox();
        createPortal();

        staticNode = Node::create("staticNode", rootNode);

        createLightSource();
        createTestChamber();
    }

    void createShaders() {
        solidColorShader = Engine::instance().shaderManager()->createShaderProgram(
            "shaders/solid-color/vertex.glsl",
            "shaders/solid-color/fragment.glsl",
            "solid color shader program"
        );

        shadedSolidColorShader = Engine::instance().shaderManager()->createShaderProgram(
            "shaders/shaded-solid-color/vertex.glsl",
            "shaders/shaded-solid-color/fragment.glsl",
            "shaded solid color shader program"
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

    void createPlaceholderMaterials() {
        // Reuse existing resources for now. Swap these later for your kit textures.
        wallDiffuseTexture = Engine::instance().textureManager()->getTexture("wall.jpg", true);
        wallSpecularTexture = Engine::instance().textureManager()->getTexture("specular.png", false);

        floorDiffuseTexture = Engine::instance().textureManager()->getTexture("diffuse.png", true);
        floorSpecularTexture = Engine::instance().textureManager()->getTexture("specular.png", false);
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

    void createPortal() {
        portal = Portal::create(camera);

        // Put portals on two “portalable” wall panels (Room A north wall, Room B south wall).
        // These are tuned for the layout created in createTestChamber().
        portal->portal1Node->transform()->setPosition(0.0f, -3.0f, 5.05f);
        portal->portal1Node->transform()->rotate(glm::angleAxis(glm::radians(180.f), glm::vec3(0.f, 1.0f, 0.0f)));

        portal->portal2Node->transform()->setPosition(0.0f, -3.0f, -31.05f);
        // portal2 faces +Z by default (good for south wall of Room B)

        const auto planeMesh = meshManager->loadMeshData("plane.obj");
        portal->setPortalMesh(1, planeMesh);
        portal->setPortalMesh(2, planeMesh);

        // Make plane vertical by orienting the portal child meshes (same trick as BasicDemo)
        glm::quat orientation = glm::quat(1, 0, 0, 0);
        orientation = glm::angleAxis(glm::radians(90.f), glm::vec3(1.0f, 0, 0)) * orientation;

        portal->portal1Node->getChild("childNode")->transform()->setOrientation(orientation);
        portal->portal2Node->getChild("childNode")->transform()->setOrientation(orientation);

        // Border coloring
        auto p1BorderNode = portal->portal1Node->getChild("childNode")->getChild("borderNode");
        p1BorderNode->transform()->scaleBy(1.05f);
        auto p1BorderMesh = p1BorderNode->getComponent<MeshComponent>();
        p1BorderMesh->setShader(solidColorShader);
        p1BorderMesh->setBeforeDrawCallback([](const auto& shader) {
            shader->setUniform("color", 0.05f, 0.15f, 1.f);
        });

        auto p2BorderNode = portal->portal2Node->getChild("childNode")->getChild("borderNode");
        p2BorderNode->transform()->scaleBy(1.05f);
        auto p2BorderMesh = p2BorderNode->getComponent<MeshComponent>();
        p2BorderMesh->setShader(solidColorShader);
        p2BorderMesh->setBeforeDrawCallback([](const auto& shader) {
            shader->setUniform("color", 1.f, 0.05f, 0.05f);
        });
    }

    void createLightSource() {
        // Small emissive-ish cube + lights
        auto node = meshManager->createNodeFromMeshData("cube.obj", rootNode);
        node->name = "lightSourceCube";
        node->transform()->setScale(0.1f);
        node->transform()->setPosition(5, 1, 0);

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
        pointLight->distance = 25.0f;

        const auto directLight = DirectLight::create(node);
        directLight->ambient = glm::vec3(0.05);
        directLight->diffuse = glm::vec3(0.25);
        directLight->specular = glm::vec3(0.04);
    }

    // --------- Level layout (2m grid, placeholders) ---------

    void createTestChamber() {
        const float floorY = -5.0f;

        buildRoomA(glm::vec3(0.f, floorY, 0.f));
        buildCorridorToRoomB(glm::vec3(0.f, floorY, 0.f));
        buildRoomB(glm::vec3(0.f, floorY, -24.f));
        buildRoomC(glm::vec3(0.f, floorY, -40.f));
    }

    void buildRoomA(const glm::vec3& origin) {
        // Room A: 10x10m floor (5x5 tiles of 2m), enclosed walls with a doorway on south side.
        const glm::quat qIdentity = glm::quat(1, 0, 0, 0);
        const float tileSize = 2.0f;
        const glm::vec3 tileScale(tileSize, 0.2f, tileSize);

        // floor tiles centers at [-4,-2,0,2,4]
        for (int ix = -2; ix <= 2; ix++) {
            for (int iz = -2; iz <= 2; iz++) {
                const glm::vec3 pos(origin.x + ix * tileSize, origin.y, origin.z + iz * tileSize);
                auto tile = spawnVisualCube("RoomA_FloorTile", pos, tileScale, qIdentity, staticNode);
                applyTexturedMaterialToSubtree(tile, floorDiffuseTexture, floorSpecularTexture);
                spawnStaticBoxCollider(tile, tileScale * 0.5f);
            }
        }

        // walls: panels 2m wide, 4m tall, 0.2m thick
        const glm::vec3 wallScale(2.0f, 4.0f, 0.2f);
        const float wallY = origin.y + 2.0f;   // match BasicDemo (-3 when floor y=-5)
        const float halfRoom = 5.0f;
        const float thicknessHalf = wallScale.z * 0.5f; // 0.1

        // North wall (z = +5.1) fully closed
        for (int i = -2; i <= 2; i++) {
            glm::vec3 pos(origin.x + i * tileSize, wallY, origin.z + halfRoom + thicknessHalf);
            auto w = spawnVisualCube("RoomA_Wall_N", pos, wallScale, qIdentity, staticNode);
            applyTexturedMaterialToSubtree(w, wallDiffuseTexture, wallSpecularTexture);
            spawnStaticBoxCollider(w, wallScale * 0.5f);
        }

        // South wall (z = -5.1) with a doorway in the center (skip i=0)
        for (int i = -2; i <= 2; i++) {
            if (i == 0) continue;
            glm::vec3 pos(origin.x + i * tileSize, wallY, origin.z - halfRoom - thicknessHalf);
            auto w = spawnVisualCube("RoomA_Wall_S", pos, wallScale, qIdentity, staticNode);
            applyTexturedMaterialToSubtree(w, wallDiffuseTexture, wallSpecularTexture);
            spawnStaticBoxCollider(w, wallScale * 0.5f);
        }

        // East/West walls: rotate 90deg around Y
        glm::quat qRotY90 = glm::angleAxis(glm::radians(90.f), glm::vec3(0, 1, 0));

        for (int i = -2; i <= 2; i++) {
            glm::vec3 posE(origin.x + halfRoom + thicknessHalf, wallY, origin.z + i * tileSize);
            auto e = spawnVisualCube("RoomA_Wall_E", posE, wallScale, qRotY90, staticNode);
            applyTexturedMaterialToSubtree(e, wallDiffuseTexture, wallSpecularTexture);
            spawnStaticBoxCollider(e, wallScale * 0.5f);

            glm::vec3 posW(origin.x - halfRoom - thicknessHalf, wallY, origin.z + i * tileSize);
            auto w = spawnVisualCube("RoomA_Wall_W", posW, wallScale, qRotY90, staticNode);
            applyTexturedMaterialToSubtree(w, wallDiffuseTexture, wallSpecularTexture);
            spawnStaticBoxCollider(w, wallScale * 0.5f);
        }

        // Portalable panel emphasis: make the center of north wall “clean”
        // (still same placeholder mesh; later you’ll swap to a dedicated portalable panel OBJ)
    }

    void buildCorridorToRoomB(const glm::vec3& origin) {
        // A simple corridor extending south from Room A doorway towards Room B.
        const glm::quat qIdentity = glm::quat(1, 0, 0, 0);
        const float tileSize = 2.0f;

        // corridor width: 4m (2 tiles across), length ~16m
        const glm::vec3 tileScale(tileSize, 0.2f, tileSize);

        // start just outside Room A south wall (Room A boundary at z=-5)
        // place tiles at z = -6, -8, ... -20
        for (int iz = 3; iz <= 10; iz++) {
            float z = origin.z - iz * tileSize;
            for (int ix = -1; ix <= 1; ix++) {
                // keep it 4m wide: ix=-1,0,1 gives 6m; we’ll narrow by skipping edges occasionally
                if (ix == 1) continue; // 4m wide: ix=-1,0
                glm::vec3 pos(origin.x + ix * tileSize, origin.y, z);
                auto tile = spawnVisualCube("Corridor_FloorTile", pos, tileScale, qIdentity, staticNode);
                applyTexturedMaterialToSubtree(tile, floorDiffuseTexture, floorSpecularTexture);
                spawnStaticBoxCollider(tile, tileScale * 0.5f);
            }
        }
    }

    void buildRoomB(const glm::vec3& origin) {
        // Room B: 14x14m with central pit (skip center 3x3 tiles), plus 2-step ascent to a ledge.
        const glm::quat qIdentity = glm::quat(1, 0, 0, 0);
        const float tileSize = 2.0f;

        const glm::vec3 tileScale(tileSize, 0.2f, tileSize);
        const glm::vec3 wallScale(2.0f, 4.0f, 0.2f);
        const float wallY = origin.y + 2.0f;

        // floor ring tiles at offsets [-6,-4,-2,0,2,4,6]
        for (int ix = -3; ix <= 3; ix++) {
            for (int iz = -3; iz <= 3; iz++) {
                bool inPit = (std::abs(ix) <= 1) && (std::abs(iz) <= 1);
                if (inPit) continue;

                glm::vec3 pos(origin.x + ix * tileSize, origin.y, origin.z + iz * tileSize);
                auto tile = spawnVisualCube("RoomB_FloorTile", pos, tileScale, qIdentity, staticNode);
                applyTexturedMaterialToSubtree(tile, floorDiffuseTexture, floorSpecularTexture);
                spawnStaticBoxCollider(tile, tileScale * 0.5f);
            }
        }

        // perimeter walls (7 panels each side)
        const float halfRoom = 7.0f;
        const float thicknessHalf = wallScale.z * 0.5f;

        // North/South
        for (int i = -3; i <= 3; i++) {
            glm::vec3 posN(origin.x + i * tileSize, wallY, origin.z + halfRoom + thicknessHalf);
            auto n = spawnVisualCube("RoomB_Wall_N", posN, wallScale, qIdentity, staticNode);
            applyTexturedMaterialToSubtree(n, wallDiffuseTexture, wallSpecularTexture);
            spawnStaticBoxCollider(n, wallScale * 0.5f);

            glm::vec3 posS(origin.x + i * tileSize, wallY, origin.z - halfRoom - thicknessHalf);
            auto s = spawnVisualCube("RoomB_Wall_S", posS, wallScale, qIdentity, staticNode);
            applyTexturedMaterialToSubtree(s, wallDiffuseTexture, wallSpecularTexture);
            spawnStaticBoxCollider(s, wallScale * 0.5f);
        }

        // East/West
        glm::quat qRotY90 = glm::angleAxis(glm::radians(90.f), glm::vec3(0, 1, 0));
        for (int i = -3; i <= 3; i++) {
            glm::vec3 posE(origin.x + halfRoom + thicknessHalf, wallY, origin.z + i * tileSize);
            auto e = spawnVisualCube("RoomB_Wall_E", posE, wallScale, qRotY90, staticNode);
            applyTexturedMaterialToSubtree(e, wallDiffuseTexture, wallSpecularTexture);
            spawnStaticBoxCollider(e, wallScale * 0.5f);

            glm::vec3 posW(origin.x - halfRoom - thicknessHalf, wallY, origin.z + i * tileSize);
            auto w = spawnVisualCube("RoomB_Wall_W", posW, wallScale, qRotY90, staticNode);
            applyTexturedMaterialToSubtree(w, wallDiffuseTexture, wallSpecularTexture);
            spawnStaticBoxCollider(w, wallScale * 0.5f);
        }

        // Two-step ascent (simple, collider-friendly)
        // Step blocks: 2x1x2, stacked to reach a ledge at y=-3.
        const glm::vec3 stepScale(2.0f, 1.0f, 2.0f);

        // Step 1: top at -3.9, Step 2: top at -2.9 (matches ledge tile top)
        auto step1 = spawnVisualCube(
            "RoomB_Step1",
            glm::vec3(origin.x - 6.0f, origin.y + 0.6f, origin.z + 4.0f), // center y: -4.4
            stepScale,
            qIdentity,
            staticNode
        );
        applyTexturedMaterialToSubtree(step1, floorDiffuseTexture, floorSpecularTexture);
        spawnStaticBoxCollider(step1, stepScale * 0.5f);

        auto step2 = spawnVisualCube(
            "RoomB_Step2",
            glm::vec3(origin.x - 6.0f, origin.y + 1.6f, origin.z + 2.0f), // center y: -3.4
            stepScale,
            qIdentity,
            staticNode
        );
        applyTexturedMaterialToSubtree(step2, floorDiffuseTexture, floorSpecularTexture);
        spawnStaticBoxCollider(step2, stepScale * 0.5f);

        // Ledge platform: 2 tiles at y=-3 (center), near north wall
        const float ledgeY = origin.y + 2.0f; // -3
        for (int ix = -3; ix <= -2; ix++) {
            glm::vec3 pos(origin.x + ix * 2.0f, ledgeY, origin.z + 6.0f);
            auto tile = spawnVisualCube("RoomB_LedgeTile", pos, tileScale, qIdentity, staticNode);
            applyTexturedMaterialToSubtree(tile, floorDiffuseTexture, floorSpecularTexture);
            spawnStaticBoxCollider(tile, tileScale * 0.5f);
        }
    }

    void buildRoomC(const glm::vec3& origin) {
        // Room C: simple traversal corridor segment with pillars to create depth cues.
        const glm::quat qIdentity = glm::quat(1, 0, 0, 0);
        const float tileSize = 2.0f;

        const glm::vec3 tileScale(tileSize, 0.2f, tileSize);
        const glm::vec3 pillarScale(0.5f, 4.0f, 0.5f);

        // long floor strip (4m wide, 24m long) starting at origin.z
        for (int iz = 0; iz < 12; iz++) {
            float z = origin.z - iz * tileSize;
            for (int ix = -1; ix <= 0; ix++) {
                glm::vec3 pos(origin.x + ix * tileSize, origin.y, z);
                auto tile = spawnVisualCube("RoomC_FloorTile", pos, tileScale, qIdentity, staticNode);
                applyTexturedMaterialToSubtree(tile, floorDiffuseTexture, floorSpecularTexture);
                spawnStaticBoxCollider(tile, tileScale * 0.5f);
            }
        }

        // pillars every 6m
        for (int i = 1; i <= 3; i++) {
            float z = origin.z - i * 6.0f;
            glm::vec3 posL(origin.x - 2.5f, origin.y + 2.0f, z);
            auto pL = spawnVisualCube("RoomC_PillarL", posL, pillarScale, qIdentity, staticNode);
            applyTexturedMaterialToSubtree(pL, wallDiffuseTexture, wallSpecularTexture);
            spawnStaticBoxCollider(pL, pillarScale * 0.5f);

            glm::vec3 posR(origin.x + 0.5f, origin.y + 2.0f, z);
            auto pR = spawnVisualCube("RoomC_PillarR", posR, pillarScale, qIdentity, staticNode);
            applyTexturedMaterialToSubtree(pR, wallDiffuseTexture, wallSpecularTexture);
            spawnStaticBoxCollider(pR, pillarScale * 0.5f);
        }
    }
};