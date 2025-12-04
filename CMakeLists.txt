cmake_minimum_required(VERSION 4.0)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(CMAKE_CXX_STANDARD 20)
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
    managers/window_manager.cpp
    managers/window_manager.h
    managers/shader_manager.cpp
    managers/shader_manager.h
    managers/mesh_manager.cpp
    managers/mesh_manager.h
    managers/texture_manager.cpp
    managers/texture_manager.h
    managers/physics_manager.cpp
    managers/physics_manager.h
    managers/metrics_manager.cpp
    managers/metrics_manager.h
    entities/window.cpp
    entities/window.h
    entities/input.cpp
    entities/input.h
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
    entities/components/free_controller.cpp
    entities/components/free_controller.h
    entities/components/mesh.cpp
    entities/components/mesh.h
    entities/components/light.cpp
    entities/components/light.h
    entities/components/rigid_body.cpp
    entities/components/rigid_body.h
    render-pipeline/framebuffers/base_frame_buffer.cpp
    render-pipeline/framebuffers/base_frame_buffer.h
    render-pipeline/framebuffers/screen_frame_buffer.cpp
    render-pipeline/framebuffers/screen_frame_buffer.h
    render-pipeline/framebuffers/msaa_frame_buffer.cpp
    render-pipeline/framebuffers/msaa_frame_buffer.h
    render-pipeline/portal/portal.cpp
    render-pipeline/portal/portal.h
    helpers/errors.cpp
    helpers/errors.h
    helpers/node_logger.cpp
    helpers/node_logger.h
    helpers/converter.cpp
    helpers/converter.h

    demos/basic_demo.h
)

target_link_libraries(main PRIVATE
    glfw
    glad::glad
    glm::glm
    assimp::assimp
    ${BULLET_LIBRARIES}
)


