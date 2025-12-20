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
    struct Material2Tex {
        std::shared_ptr<Texture> albedo;
        std::shared_ptr<Texture> specular;
    };

    struct Materials {
        Material2Tex wall;
        Material2Tex floor;
        Material2Tex accent;
    };

    std::shared_ptr<Node> rootNode;
    std::shared_ptr<Node> staticNode;

    std::shared_ptr<MeshManager> meshManager;
    std::shared_ptr<btDynamicsWorld> dynamicsWorld;

    std::shared_ptr<ShaderProgram> solidColorShader;
    std::shared_ptr<ShaderProgram> shadedSolidColorShader;
    std::shared_ptr<ShaderProgram> blinnPhongShader;
    std::shared_ptr<ShaderProgram> skyboxShader;

    std::shared_ptr<Texture> skyboxTexture;

    // ------------------------------
    // Materials / texture loading
    // ------------------------------
    //
    // Texture requirements for this engine path (Blinn-Phong):
    // - Supported file types: whatever stb_image decodes (PNG/JPG are recommended).
    // - Meshes MUST have UVs + normals (importer requires both).
    // - Channels: prefer 3-channel RGB for albedo/specular. Alpha isn’t part of this lighting pipeline.
    // - Color space:
    //   - Albedo should be loaded with isAlbedo=true (engine stores it as sRGB).
    //   - Specular should be loaded with isAlbedo=false (linear).
    // - UV tiling: sampler wrap is REPEAT, so UVs may exceed 0–1 to tile.
    Materials m_materials;

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

    void applyTexturedMaterialToSubtree(const std::shared_ptr<Node>& node, const Material2Tex& material) {
        auto subtreeMeshes = node->getChildComponents<MeshComponent>();

        // If the root itself has a mesh (common for cube.obj), include it too.
        if (auto rootMesh = node->getComponent<MeshComponent>()) {
            subtreeMeshes.push_back(rootMesh);
        }

        for (auto& m : subtreeMeshes) {
            m->setShader(blinnPhongShader);
            m->setBeforeDrawCallback([material](const std::shared_ptr<ShaderProgram>& s) {
                s->setTexture("diffuseTexture", material.albedo);
                s->setTexture("specularTexture", material.specular);
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

    void spawnDoorFrameMarker(
        const std::string& namePrefix,
        const glm::vec3& centerBottom,
        float width,
        float height,
        float thickness,
        const glm::quat& orientation,
        const Material2Tex& material
    ) {
        // Visual-only marker to make openings obvious while we prototype with cubes.
        const glm::quat q = orientation;
        const float postThickness = thickness;
        const float beamThickness = thickness;

        const glm::vec3 postScale(postThickness, height, postThickness);
        const glm::vec3 beamScale(width, beamThickness, postThickness);

        const glm::vec3 leftPos = centerBottom + glm::vec3(-width * 0.5f, height * 0.5f, 0.f);
        const glm::vec3 rightPos = centerBottom + glm::vec3(width * 0.5f, height * 0.5f, 0.f);
        const glm::vec3 beamPos = centerBottom + glm::vec3(0.f, height - beamThickness * 0.5f, 0.f);

        auto left = spawnVisualCube(namePrefix + "_Left", leftPos, postScale, q, staticNode);
        applyTexturedMaterialToSubtree(left, material);

        auto right = spawnVisualCube(namePrefix + "_Right", rightPos, postScale, q, staticNode);
        applyTexturedMaterialToSubtree(right, material);

        auto beam = spawnVisualCube(namePrefix + "_Top", beamPos, beamScale, q, staticNode);
        applyTexturedMaterialToSubtree(beam, material);
    }

    std::shared_ptr<Node> spawnPointLight(
        const std::string& name,
        const glm::vec3& position,
        const glm::vec3& color,
        float distance
    ) {
        // Visible marker cube + PointLight component.
        auto node = meshManager->createNodeFromMeshData("cube.obj", rootNode);
        node->name = name;
        node->transform()->setPosition(position);
        node->transform()->setScale(0.12f);

        auto mesh = node->getComponent<MeshComponent>();
        mesh->setShader(solidColorShader);
        mesh->setBeforeDrawCallback([color](const std::shared_ptr<ShaderProgram>& shaderProgram) {
            shaderProgram->setUniform("color", color);
        });
        meshes.push_back(mesh);

        const auto pointLight = PointLight::create(node);
        pointLight->ambient = color * 0.02f;
        pointLight->diffuse = color * 0.6f;
        pointLight->specular = color * 0.2f;
        pointLight->distance = distance;

        return node;
    }

    void spawnPointLightsLine(
        const std::string& namePrefix,
        const glm::vec3& start,
        const glm::vec3& step,
        int count,
        const glm::vec3& color,
        float distance
    ) {
        for (int i = 0; i < count; i++) {
            spawnPointLight(
                namePrefix + "_" + std::to_string(i),
                start + static_cast<float>(i) * step,
                color,
                distance
            );
        }
    }

    void spawnPointLightsGrid(
        const std::string& namePrefix,
        const glm::vec3& origin,
        const glm::vec3& stepX,
        const glm::vec3& stepZ,
        int countX,
        int countZ,
        const glm::vec3& color,
        float distance
    ) {
        for (int iz = 0; iz < countZ; iz++) {
            for (int ix = 0; ix < countX; ix++) {
                const glm::vec3 pos = origin + static_cast<float>(ix) * stepX + static_cast<float>(iz) * stepZ;
                spawnPointLight(
                    namePrefix + "_x" + std::to_string(ix) + "_z" + std::to_string(iz),
                    pos,
                    color,
                    distance
                );
            }
        }
    }

    // --------- Scene setup ---------

    void createScene() {
        createShaders();

        // Player camera/controller
        createPlayer();

        // Materials/textures
        loadMaterials();

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

    void loadMaterials() {
        // Update these paths to point at your real kit textures in resources/.
        // Suggested layout:
        // - resources/kit/wall_albedo.png, resources/kit/wall_specular.png
        // - resources/kit/floor_albedo.png, resources/kit/floor_specular.png
        //
        // Fallback behavior: if a kit texture is missing, we fall back to existing repo textures.
        const auto tm = Engine::instance().textureManager();

        try {
            m_materials.wall.albedo = tm->getTexture("kit/wall_albedo.png", true);
            m_materials.wall.specular = tm->getTexture("kit/wall_specular.png", false);
        } catch (...) {
            m_materials.wall.albedo = tm->getTexture("wall.jpg", true);
            m_materials.wall.specular = tm->getTexture("specular.png", false);
        }

        try {
            m_materials.floor.albedo = tm->getTexture("kit/floor_albedo.png", true);
            m_materials.floor.specular = tm->getTexture("kit/floor_specular.png", false);
        } catch (...) {
            m_materials.floor.albedo = tm->getTexture("diffuse.png", true);
            m_materials.floor.specular = tm->getTexture("specular.png", false);
        }

        // Accent defaults to wall for now.
        m_materials.accent = m_materials.wall;
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
        directLight->diffuse = glm::vec3(0.05);
        directLight->specular = glm::vec3(0.04);
    }

    // --------- Level layout (2m grid, placeholders) ---------

    // Central place for tweaking room/corridor coordinates.
    struct LevelLayout {
        float floorY = -5.0f;
        float tileSize = 2.0f;
        float tileThickness = 0.2f;

        float wallHeight = 4.0f;
        float wallThickness = 0.2f;

        // Door width in wall panels (panel width == tileSize).
        // doorPanels=2 => 4m opening (skip i=-1 and i=0 in the wall loop).
        int doorPanels = 2;

        // Room half-extents (meters)
        float roomAHalf = 5.0f; // 10m room
        float roomBHalf = 7.0f; // 14m room

        // Corridor geometry
        int corridorWidthTiles = 2; // 4m wide
        int corridorABTiles = 8;    // 16m long
        int corridorBCTiles = 4;    // 8m long

        glm::vec3 roomAOrigin{};
        glm::vec3 roomBOrigin{};
        glm::vec3 roomCOrigin{};
    };

    void createTestChamber() {
        // --- Level layout coordinates (2m grid) ---
        LevelLayout L{};
        L.roomAOrigin = glm::vec3(0.f, L.floorY, 0.f);
        L.roomBOrigin = glm::vec3(0.f, L.floorY, -24.f);
        L.roomCOrigin = glm::vec3(0.f, L.floorY, -40.f);

        createSafetyGround(L);

        buildRoomA(L, L.roomAOrigin);
        buildCorridorAtoB(L);
        buildRoomB(L, L.roomBOrigin);
        buildCorridorBtoC(L);
        buildRoomC(L, L.roomCOrigin - 5.0f);

        placeTestChamberLights(L);

        positionPortals(L);
    }

    void placeTestChamberLights(const LevelLayout& L) {
        // Easy knobs:
        // - change count/spacing to extend the lit area
        // - use grid in larger rooms for even lighting
        const float ceilingY = L.floorY + L.wallHeight - 0.5f;

        // const std::string& namePrefix,
        // const glm::vec3& start,
        // const glm::vec3& step,
        // int count,
        // const glm::vec3& color,
        // float distance
        // A line of “ceiling” lights along the main Z direction (towards Room C).
        // spawnPointLightsLine(
        //     "CeilingLight",
        //     glm::vec3(-1.0f, ceilingY + 10.0f, L.roomAOrigin.z + 1.0f),
        //     glm::vec3(0.0f, 0.0f, -30.0f),
        //     5,
        //     glm::vec3(1.0f, 1.0f, 1.0f),
        //     100.0f
        // );

        // Small grid in Room B for better readability around the pit.
        // spawnPointLightsGrid(
        //     "RoomB_LightGrid",
        //     glm::vec3(-3.0f, ceilingY, L.roomBOrigin.z + 3.0f),
        //     glm::vec3(6.0f, 0.0f, 0.0f),
        //     glm::vec3(0.0f, 0.0f, -6.0f),
        //     2,
        //     2,
        //     glm::vec3(1.0f, 0.95f, 0.9f),
        //     22.0f
        // );
    }

    void createSafetyGround(const LevelLayout& L) {
        // Big slab under the entire level so you never fall infinitely while iterating.
        const glm::quat qIdentity = glm::quat(1, 0, 0, 0);

        const glm::vec3 scale(100.0f, 2.0f, 100.0f);
        const glm::vec3 position(0.0f, L.floorY - 2.0f, -30.0f);

        auto slab = spawnVisualCube("SafetyGround", position, scale, qIdentity, staticNode);
        applyTexturedMaterialToSubtree(slab, m_materials.floor);
        spawnStaticBoxCollider(slab, scale * 0.5f);
    }

    void positionPortals(const LevelLayout& L) const {
        const float y = L.roomAOrigin.y + L.wallHeight * 0.5f;

        // Portal 1: Room A north wall, facing into the room (-Z)
        portal->portal1Node->transform()->setPosition(
            L.roomAOrigin.x,
            y,
            L.roomAOrigin.z + L.roomAHalf + L.wallThickness * 0.5f + 0.05f
        );
        portal->portal1Node->transform()->setOrientation(glm::quat(1, 0, 0, 0));
        portal->portal1Node->transform()->rotate(glm::angleAxis(glm::radians(180.f), glm::vec3(0.f, 1.0f, 0.0f)));

        // Portal 2: Room B south wall, facing into Room B (+Z)
        portal->portal2Node->transform()->setPosition(
            L.roomBOrigin.x,
            L.roomBOrigin.y + L.wallHeight * 0.5f,
            L.roomBOrigin.z - L.roomBHalf - L.wallThickness * 0.5f - 0.05f
        );
        portal->portal2Node->transform()->setOrientation(glm::quat(1, 0, 0, 0));
    }

    void buildRoomA(const LevelLayout& L, const glm::vec3& origin) {
        const glm::quat qIdentity = glm::quat(1, 0, 0, 0);
        const float tileSize = L.tileSize;
        const glm::vec3 tileScale(tileSize, L.tileThickness, tileSize);

        // floor tiles (10m x 10m)
        for (int ix = -2; ix <= 2; ix++) {
            for (int iz = -2; iz <= 2; iz++) {
                const glm::vec3 pos(origin.x + ix * tileSize, origin.y, origin.z + iz * tileSize);
                auto tile = spawnVisualCube("RoomA_FloorTile", pos, tileScale, qIdentity, staticNode);
                applyTexturedMaterialToSubtree(tile, m_materials.floor);
                spawnStaticBoxCollider(tile, tileScale * 0.5f);
            }
        }

        // walls: panels 2m wide, 4m tall, 0.2m thick
        const glm::vec3 wallScale(tileSize, L.wallHeight, L.wallThickness);
        const float wallY = origin.y + L.wallHeight * 0.5f;
        const float halfRoom = L.roomAHalf;
        const float thicknessHalf = wallScale.z * 0.5f;

        // North wall fully closed
        for (int i = -2; i <= 2; i++) {
            glm::vec3 pos(origin.x + i * tileSize, wallY, origin.z + halfRoom + thicknessHalf);
            auto w = spawnVisualCube("RoomA_Wall_N", pos, wallScale, qIdentity, staticNode);
            applyTexturedMaterialToSubtree(w, m_materials.wall);
            spawnStaticBoxCollider(w, wallScale * 0.5f);
        }

        // South wall: doorway in the center (4m opening)
        for (int i = -2; i <= 2; i++) {
            if (L.doorPanels == 2 && (i == -1 || i == 0)) continue;
            glm::vec3 pos(origin.x + i * tileSize, wallY, origin.z - halfRoom - thicknessHalf);
            auto w = spawnVisualCube("RoomA_Wall_S", pos, wallScale, qIdentity, staticNode);
            applyTexturedMaterialToSubtree(w, m_materials.wall);
            spawnStaticBoxCollider(w, wallScale * 0.5f);
        }

        // East/West walls: rotate 90deg around Y
        glm::quat qRotY90 = glm::angleAxis(glm::radians(90.f), glm::vec3(0, 1, 0));
        for (int i = -2; i <= 2; i++) {
            glm::vec3 posE(origin.x + halfRoom + thicknessHalf, wallY, origin.z + i * tileSize);
            auto e = spawnVisualCube("RoomA_Wall_E", posE, wallScale, qRotY90, staticNode);
            applyTexturedMaterialToSubtree(e, m_materials.wall);
            spawnStaticBoxCollider(e, wallScale * 0.5f);

            glm::vec3 posW(origin.x - halfRoom - thicknessHalf, wallY, origin.z + i * tileSize);
            auto w = spawnVisualCube("RoomA_Wall_W", posW, wallScale, qRotY90, staticNode);
            applyTexturedMaterialToSubtree(w, m_materials.wall);
            spawnStaticBoxCollider(w, wallScale * 0.5f);
        }

        // Visual door marker (helps ensure openings are obvious)
        spawnDoorFrameMarker(
            "RoomA_Door_S",
            glm::vec3(origin.x - tileSize * 0.5f, origin.y, origin.z - halfRoom - thicknessHalf),
            tileSize * static_cast<float>(L.doorPanels),
            L.wallHeight,
            L.wallThickness,
            qIdentity,
            m_materials.accent
        );
    }

    void buildCorridorAtoB(const LevelLayout& L) {
        const glm::quat qIdentity = glm::quat(1, 0, 0, 0);
        const float tileSize = L.tileSize;
        const glm::vec3 tileScale(tileSize, L.tileThickness, tileSize);

        const glm::vec3 origin = L.roomAOrigin;
        const float startZ = origin.z - (L.roomAHalf + tileSize * 0.5f);

        for (int iz = 0; iz < L.corridorABTiles; iz++) {
            float z = startZ - iz * tileSize;
            for (int ix = -1; ix <= 0; ix++) { // 4m wide corridor aligned with 4m door
                glm::vec3 pos(origin.x + ix * tileSize, origin.y, z);
                auto tile = spawnVisualCube("CorridorAB_FloorTile", pos, tileScale, qIdentity, staticNode);
                applyTexturedMaterialToSubtree(tile, m_materials.floor);
                spawnStaticBoxCollider(tile, tileScale * 0.5f);
            }
        }
    }

    void buildCorridorBtoC(const LevelLayout& L) {
        const glm::quat qIdentity = glm::quat(1, 0, 0, 0);
        const float tileSize = L.tileSize;
        const glm::vec3 tileScale(tileSize, L.tileThickness, tileSize);

        const glm::vec3 origin = L.roomBOrigin;
        const float startZ = origin.z - (L.roomBHalf + tileSize * 0.5f);

        for (int iz = 0; iz < L.corridorBCTiles; iz++) {
            float z = startZ - (iz + 1) * tileSize;
            for (int ix = -1; ix <= 0; ix++) {
                glm::vec3 pos(origin.x + ix * tileSize, origin.y, z);
                auto tile = spawnVisualCube("CorridorBC_FloorTile", pos, tileScale, qIdentity, staticNode);
                applyTexturedMaterialToSubtree(tile, m_materials.floor);
                spawnStaticBoxCollider(tile, tileScale * 0.5f);
            }
        }
    }

    void buildRoomB(const LevelLayout& L, const glm::vec3& origin) {
        const glm::quat qIdentity = glm::quat(1, 0, 0, 0);
        const float tileSize = L.tileSize;

        const glm::vec3 tileScale(tileSize, L.tileThickness, tileSize);
        const glm::vec3 wallScale(tileSize, L.wallHeight, L.wallThickness);
        const float wallY = origin.y + L.wallHeight * 0.5f;

        // floor ring tiles at offsets [-6,-4,-2,0,2,4,6]
        for (int ix = -3; ix <= 3; ix++) {
            for (int iz = -3; iz <= 3; iz++) {
                bool inPit = (std::abs(ix) <= 1) && (std::abs(iz) <= 1);
                if (inPit) continue;

                glm::vec3 pos(origin.x + ix * tileSize, origin.y, origin.z + iz * tileSize);
                auto tile = spawnVisualCube("RoomB_FloorTile", pos, tileScale, qIdentity, staticNode);
                applyTexturedMaterialToSubtree(tile, m_materials.floor);
                spawnStaticBoxCollider(tile, tileScale * 0.5f);
            }
        }

        // perimeter walls (7 panels each side)
        const float halfRoom = L.roomBHalf;
        const float thicknessHalf = wallScale.z * 0.5f;

        // North wall opening to corridor AB, South wall opening to corridor BC
        for (int i = -3; i <= 3; i++) {
            // north
            if (!(L.doorPanels == 2 && (i == -1 || i == 0))) {
                glm::vec3 posN(origin.x + i * tileSize, wallY, origin.z + halfRoom + thicknessHalf);
                auto n = spawnVisualCube("RoomB_Wall_N", posN, wallScale, qIdentity, staticNode);
                applyTexturedMaterialToSubtree(n, m_materials.wall);
                spawnStaticBoxCollider(n, wallScale * 0.5f);
            }

            // south
            if (!(L.doorPanels == 2 && (i == -1 || i == 0))) {
                glm::vec3 posS(origin.x + i * tileSize, wallY, origin.z - halfRoom - thicknessHalf);
                auto s = spawnVisualCube("RoomB_Wall_S", posS, wallScale, qIdentity, staticNode);
                applyTexturedMaterialToSubtree(s, m_materials.wall);
                spawnStaticBoxCollider(s, wallScale * 0.5f);
            }
        }

        // Visual door markers (north and south openings)
        spawnDoorFrameMarker(
            "RoomB_Door_N",
            glm::vec3(origin.x - tileSize * 0.5f, origin.y, origin.z + halfRoom + thicknessHalf),
            tileSize * static_cast<float>(L.doorPanels),
            L.wallHeight,
            L.wallThickness,
            qIdentity,
            m_materials.accent
        );

        spawnDoorFrameMarker(
            "RoomB_Door_S",
            glm::vec3(origin.x - tileSize * 0.5f, origin.y, origin.z - halfRoom - thicknessHalf),
            tileSize * static_cast<float>(L.doorPanels),
            L.wallHeight,
            L.wallThickness,
            qIdentity,
            m_materials.accent
        );

        // East/West
        glm::quat qRotY90 = glm::angleAxis(glm::radians(90.f), glm::vec3(0, 1, 0));
        for (int i = -3; i <= 3; i++) {
            glm::vec3 posE(origin.x + halfRoom + thicknessHalf, wallY, origin.z + i * tileSize);
            auto e = spawnVisualCube("RoomB_Wall_E", posE, wallScale, qRotY90, staticNode);
            applyTexturedMaterialToSubtree(e, m_materials.wall);
            spawnStaticBoxCollider(e, wallScale * 0.5f);

            glm::vec3 posW(origin.x - halfRoom - thicknessHalf, wallY, origin.z + i * tileSize);
            auto w = spawnVisualCube("RoomB_Wall_W", posW, wallScale, qRotY90, staticNode);
            applyTexturedMaterialToSubtree(w, m_materials.wall);
            spawnStaticBoxCollider(w, wallScale * 0.5f);
        }

        // Two-step ascent (simple + collider-friendly)
        const glm::vec3 stepScale(2.0f, 1.0f, 2.0f);

        auto step1 = spawnVisualCube(
            "RoomB_Step1",
            glm::vec3(origin.x - 6.0f, origin.y + 0.6f, origin.z + 4.0f),
            stepScale,
            qIdentity,
            staticNode
        );
        applyTexturedMaterialToSubtree(step1, m_materials.floor);
        spawnStaticBoxCollider(step1, stepScale * 0.5f);

        auto step2 = spawnVisualCube(
            "RoomB_Step2",
            glm::vec3(origin.x - 6.0f, origin.y + 1.6f, origin.z + 2.0f),
            stepScale,
            qIdentity,
            staticNode
        );
        applyTexturedMaterialToSubtree(step2, m_materials.floor);
        spawnStaticBoxCollider(step2, stepScale * 0.5f);

        // Ledge platform: 2 tiles at y=-3, near north wall
        const float ledgeY = origin.y + L.wallHeight * 0.5f;
        for (int ix = -3; ix <= -2; ix++) {
            glm::vec3 pos(origin.x + ix * tileSize, ledgeY, origin.z + 6.0f);
            auto tile = spawnVisualCube("RoomB_LedgeTile", pos, tileScale, qIdentity, staticNode);
            applyTexturedMaterialToSubtree(tile, m_materials.floor);
            spawnStaticBoxCollider(tile, tileScale * 0.5f);
        }
    }

    void buildRoomC(const LevelLayout& L, const glm::vec3& origin) {
        // Room C: corridor-like strip with pillars (easy movement testbed).
        const glm::quat qIdentity = glm::quat(1, 0, 0, 0);
        const float tileSize = L.tileSize;

        const glm::vec3 tileScale(tileSize, L.tileThickness, tileSize);
        const glm::vec3 pillarScale(0.5f, L.wallHeight, 0.5f);

        // long floor strip (4m wide, 24m long) starting at origin.z
        for (int iz = 0; iz < 12; iz++) {
            float z = origin.z - iz * tileSize;
            for (int ix = -1; ix <= 0; ix++) {
                glm::vec3 pos(origin.x + ix * tileSize, origin.y, z);
                auto tile = spawnVisualCube("RoomC_FloorTile", pos, tileScale, qIdentity, staticNode);
                applyTexturedMaterialToSubtree(tile, m_materials.floor);
                spawnStaticBoxCollider(tile, tileScale * 0.5f);
            }
        }

        // pillars every 6m
        for (int i = 1; i <= 3; i++) {
            float z = origin.z - i * 6.0f;
            glm::vec3 posL(origin.x - 2.5f, origin.y + L.wallHeight * 0.5f, z);
            auto pL = spawnVisualCube("RoomC_PillarL", posL, pillarScale, qIdentity, staticNode);
            applyTexturedMaterialToSubtree(pL, m_materials.wall);
            spawnStaticBoxCollider(pL, pillarScale * 0.5f);

            glm::vec3 posR(origin.x + 0.5f, origin.y + L.wallHeight * 0.5f, z);
            auto pR = spawnVisualCube("RoomC_PillarR", posR, pillarScale, qIdentity, staticNode);
            applyTexturedMaterialToSubtree(pR, m_materials.wall);
            spawnStaticBoxCollider(pR, pillarScale * 0.5f);
        }
    }
};