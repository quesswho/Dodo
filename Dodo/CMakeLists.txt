add_library(Dodo STATIC)


file(GLOB_RECURSE DODO_CPP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Core/*.cpp
)

# Add window api sources
if(WIN32)
    message(STATUS "Adding Windows-specific sources")
    file(GLOB_RECURSE DODO_WIN32_CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/WindowAPI/Win32/*.cpp)
    list(APPEND DODO_CPP_SOURCES ${DODO_WIN32_CPP_SOURCES})
else()
    file(GLOB_RECURSE DODO_GLFW_CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/WindowAPI/GLFW/*.cpp)
    list(APPEND DODO_CPP_SOURCES ${DODO_GLFW_CPP_SOURCES})
endif()

# Add render api sources
if(DD_API_OPENGL)
    file(GLOB_RECURSE DODO_OPENGL_CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/GraphicAPI/OpenGL/*.cpp)
    list(APPEND DODO_CPP_SOURCES ${DODO_OPENGL_CPP_SOURCES})
elif(DD_API_VULKAN)
    file(GLOB_RECURSE DODO_VULKAN_CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/GraphicAPI/Vulkan/*.cpp)
    list(APPEND DODO_CPP_SOURCES ${DODO_VULKAN_CPP_SOURCES})
endif()

file(GLOB_RECURSE DODO_H_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

target_sources(Dodo PRIVATE
        src/pch.cpp
        ${DODO_CPP_SOURCES}
        ${DODO_H_SOURCES}
        lib/stb_image/stb_image.cpp
)

target_include_directories(Dodo
    PUBLIC
        src
        lib/stb_image
    PRIVATE
        lib/spdlog/include
        lib/glad/include
        lib/imgui/include
        lib/assimp/include
)

target_link_libraries(Dodo
    PUBLIC
        glad
        imgui
        assimp
)

target_compile_definitions(Dodo
    PRIVATE
        _CRT_SECURE_NO_WARNINGS
)

# Precompiled headers
target_precompile_headers(Dodo PRIVATE src/pch.h)

# Platform-specific
if (WIN32)
    target_compile_definitions(Dodo PRIVATE DD_x64 DD_API_WIN32)
else ()
    target_compile_definitions(Dodo PRIVATE DD_API_GLFW)
endif()

target_compile_definitions(Dodo
    PUBLIC
        DD_MATH_COLUMN_MAJOR
        DD_COORDINATE_RIGHT_HANDED
    PRIVATE
        $<$<CONFIG:Debug>:DD_DEBUG>
        $<$<CONFIG:Release>:DD_RELEASE>
)