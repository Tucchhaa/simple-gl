cmake_minimum_required(VERSION 4.0)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(main)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Bullet CONFIG REQUIRED)

add_executable(main
    main.cpp
    managers/engine.cpp
    managers/engine.h
    managers/shader_manager.cpp
    managers/shader_manager.h
    managers/mesh_manager.cpp
    managers/mesh_manager.h
    managers/texture_manager.cpp
    managers/texture_manager.h
    managers/physics_manager.cpp
    managers/physics_manager.h
    window/window.cpp
    window/window.h
    window/input.cpp
    window/input.h
    window/framebuffers/base_frame_buffer.cpp
    window/framebuffers/base_frame_buffer.h
    window/framebuffers/screen_frame_buffer.cpp
    window/framebuffers/screen_frame_buffer.h
    window/framebuffers/msaa_frame_buffer.cpp
    window/framebuffers/msaa_frame_buffer.h
    window/window_panel.cpp
    window/window_panel.h
    entities/shader_program.cpp
    entities/shader_program.h
    entities/scene.cpp
    entities/scene.h
    entities/node.cpp
    entities/node.h
    entities/mesh_data.cpp
    entities/mesh_data.h
    entities/texture.cpp
    entities/texture.h
    entities/components/component.cpp
    entities/components/component.h
    entities/components/transform.cpp
    entities/components/transform.h
    entities/components/camera.cpp
    entities/components/camera.h
    entities/components/mesh.cpp
    entities/components/mesh.h
    entities/components/light.cpp
    entities/components/light.h
    entities/components/rigid_body.cpp
    entities/components/rigid_body.h
    entities/components/controllers/free_controller.cpp
    entities/components/controllers/free_controller.h
    entities/components/controllers/character_controller.cpp
    entities/components/controllers/character_controller.h
    entities/components/portal/portal_fps_controller.cpp
    entities/components/portal/portal_fps_controller.h
    entities/components/portal/portal_bullet.cpp
    entities/components/portal/portal_bullet.h
    entities/components/portal/teleportable.cpp
    entities/components/portal/teleportable.h
    entities/physics/contact_callback.cpp
    entities/physics/contact_callback.h
    entities/physics/sweep_callback.cpp
    entities/physics/sweep_callback.h
    render-pipeline/portal/portal.cpp
    render-pipeline/portal/portal.h
    render-pipeline/portal/portal_framebuffer.cpp
    render-pipeline/portal/portal_framebuffer.h
    helpers/node_logger.cpp
    helpers/node_logger.h
    helpers/converter.cpp
    helpers/converter.h
    helpers/quick_accessors.cpp
    helpers/quick_accessors.h

    demos/basic_demo.h
)

target_link_libraries(main PRIVATE
    glfw
    glad::glad
    glm::glm
    assimp::assimp
    ${BULLET_LIBRARIES}
)


