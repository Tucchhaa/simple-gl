cmake_minimum_required(VERSION 4.0)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(main)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

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
    entities/window.cpp
    entities/window.h
    entities/input.cpp
    entities/input.h
    entities/shader_program.cpp
    entities/shader_program.h
    entities/node.cpp
    entities/node.h
    entities/mesh_data.cpp
    entities/mesh_data.h
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
    helpers/errors.cpp
    helpers/errors.h
)

target_link_libraries(main PRIVATE
    glfw
    glad::glad
    glm::glm
    assimp::assimp
)





