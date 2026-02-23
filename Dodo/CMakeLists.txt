add_library(Dodo STATIC)


file(GLOB_RECURSE DODO_CPP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

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
endif()

target_compile_definitions(Dodo
    PUBLIC
        DD_MATH_COLUMN_MAJOR
        DD_COORDINATE_RIGHT_HANDED
    PRIVATE
        $<$<CONFIG:Debug>:DD_DEBUG>
        $<$<CONFIG:Release>:DD_RELEASE>
)