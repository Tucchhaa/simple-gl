cmake_minimum_required(VERSION 4.0)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(main)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(main
    main.cpp
    managers/window_manager.cpp
    managers/window_manager.h
    managers/shader_manager.cpp
    managers/shader_manager.h
    entities/window.cpp
    entities/window.h
    entities/shader_program.cpp
    entities/shader_program.h
)

target_link_libraries(main PRIVATE
    glfw
    glad::glad
    glm::glm
)





