cmake_minimum_required(VERSION 3.25)
project(Dodo LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Load Opengl and (or) Vulkan
option(DD_API_OPENGL "Enable OpenGL loader" ON)
option(DD_API_VULKAN "Enable Vulkan loader" OFF)


message(STATUS "=== Configuring Dodo ===")
message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")

message(STATUS "DD_GLAD_OPENGL=${DD_GLAD_OPENGL}")
message(STATUS "DD_GLAD_VULKAN=${DD_GLAD_VULKAN}")


# Output dirs
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Dependencies
message(STATUS "=== Adding GLAD ===")
add_subdirectory(Dodo/lib/glad)

message(STATUS "=== Adding ImGui ===")
add_subdirectory(Dodo/lib/imgui)

message(STATUS "=== Adding Assimp ===")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(Dodo/lib/assimp)


# Projects
message(STATUS "=== Adding Dodo ===")
add_subdirectory(Dodo)

message(STATUS "=== Adding Sandbox ===")
add_subdirectory(Sandbox)

message(STATUS "=== Adding DodoEditor ===")
add_subdirectory(DodoEditor)