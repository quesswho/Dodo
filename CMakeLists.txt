cmake_minimum_required(VERSION 3.26)
project(Dodo LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile commands, useful for IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_DEFAULT_BUILD_TYPE Debug)


# Load Opengl or Vulkan
option(DD_API_OPENGL "Enable OpenGL loader" ON)
option(DD_API_VULKAN "Enable Vulkan loader" OFF)

option(DD_API_WIN32 "Enable Win32" OFF)
option(DD_API_GLFW "Enable GLFW" ON)

option(COMPILE_EDITOR "Compile DodoEditor project" ON)
option(COMPILE_SANDBOX "Compile Sandbox project" OFF)
option(COMPILE_GAME "Compile Game project" ON)

message(STATUS "=== Configuring Dodo ===")
message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")

message(STATUS "DD_API_OPENGL=${DD_API_OPENGL}")
message(STATUS "DD_API_VULKAN=${DD_API_VULKAN}")
message(STATUS "DD_API_WIN32=${DD_API_WIN32}")
message(STATUS "DD_API_GLFW=${DD_API_GLFW}")


# Output dirs
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


include(cmake/Slang.cmake)

# Dependencies
message(STATUS "=== Configuring GLAD ===")
add_subdirectory(Dodo/lib/glad)


message(STATUS "=== Configuring ImGui ===")
add_subdirectory(Dodo/lib/imgui)

message(STATUS "=== Configuring Assimp ===")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Only build importers we actually use
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)

# Temporarily suppress ASSIMP messages
set(CMAKE_MESSAGE_LOG_LEVEL_BACKUP ${CMAKE_MESSAGE_LOG_LEVEL})
set(CMAKE_MESSAGE_LOG_LEVEL ERROR)
add_subdirectory(Dodo/lib/assimp)
set(CMAKE_MESSAGE_LOG_LEVEL ${CMAKE_MESSAGE_LOG_LEVEL_BACKUP})

# Strip debug info from Assimp to reduce binary size
target_compile_options(assimp PRIVATE -g0)

message(STATUS "=== Configuring GLFW ===")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(Dodo/lib/glfw)

# Projects
message(STATUS "=== Configuring Dodo ===")
add_subdirectory(Dodo)

if (COMPILE_EDITOR)
    message(STATUS "=== Configuring DodoEditor ===")
    add_subdirectory(DodoEditor)
endif ()

if (COMPILE_SANDBOX)
    message(STATUS "=== Configuring Sandbox ===")
    add_subdirectory(Sandbox)
endif ()

if (COMPILE_GAME)
    message(STATUS "=== Configuring Game ===")
    add_subdirectory(Game)
endif ()
