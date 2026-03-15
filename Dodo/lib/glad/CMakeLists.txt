add_library(glad STATIC)

# GLAD is written in C
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)


## SOURCES ##
set(GLAD_SOURCES src/gl.c)

# Windows OpenGL (WGL) API
if (DD_API_WIN32)
    list(APPEND GLAD_SOURCES src/wgl.c)
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


# Debug / Release flags
target_compile_options(glad
    PRIVATE
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-O3>
)