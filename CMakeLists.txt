cmake_minimum_required(VERSION 3.25)
project(MyEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output dirs
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Dependencies
add_subdirectory(Dodo/lib/glad)
add_subdirectory(Dodo/lib/imgui)
add_subdirectory(Dodo/lib/assimp)

# Projects
add_subdirectory(Dodo)
add_subdirectory(Sandbox)
add_subdirectory(Dodoeditor)