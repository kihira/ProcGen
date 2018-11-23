cmake_minimum_required(VERSION 3.12)
project(ProcGen)

set(CMAKE_CXX_STANDARD 14)

add_executable(ProcGen src/main.cpp src/Mesh.cpp src/Mesh.h src/Shader.cpp src/Shader.h src/Light.h)

# GLFW
# Disable GLFW docs, tests and examples
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw)
target_link_libraries(${PROJECT_NAME} glfw)

# GLAD
add_library(glad libs/glad/src/glad.c)
target_include_directories(glad PRIVATE libs/glad/include)
target_include_directories(${PROJECT_NAME} PRIVATE libs/glad/include)
target_link_libraries(${PROJECT_NAME} glad)

# GLM
include_directories(${PROJECT_NAME} libs/glm)
