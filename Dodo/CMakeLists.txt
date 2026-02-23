add_library(Dodo STATIC)

target_sources(Dodo
    PRIVATE
        src/pch.cpp
        src/**/*.cpp
        src/**/*.h
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
    PRIVATE
        $<$<CONFIG:Debug>:DD_DEBUG>
        $<$<CONFIG:Release>:DD_RELEASE>
)