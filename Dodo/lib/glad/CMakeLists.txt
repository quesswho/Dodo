add_library(glad STATIC)

# GLAD is written in C
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)


## SOURCES ##
set(GLAD_SOURCES src/gl.c)
if (DODO_API_VULKAN)
    list(APPEND GLAD_SOURCES src/vulkan.c)
endif()

# Windows OpenGL (WGL) API
if (WIN32)
    list(APPEND GLAD_SOURCES src/glad/wgl.c)
endif()

target_sources(glad PRIVATE ${GLAD_SOURCES})

## HEADERS ##
target_include_directories(glad
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link

# Link Opengl
find_package(OpenGL REQUIRED)
target_link_libraries(glad PUBLIC OpenGL::GL)

# Link Vulkan
if (DODO_RENDER_VULKAN)
    find_package(Vulkan REQUIRED)
    target_link_libraries(glad PUBLIC Vulkan::Vulkan)
endif()


# Debug / Release flags
target_compile_options(glad
    PRIVATE
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-O3>
)