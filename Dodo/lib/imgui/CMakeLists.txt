add_library(imgui STATIC)

target_sources(imgui PRIVATE
    src/imgui.cpp
    src/imgui_demo.cpp
    src/imgui_draw.cpp
    src/imgui_impl_opengl3.cpp
    src/imgui_impl_win32.cpp
    src/imgui_widgets.cpp
)

# We use glad for imgui opengl/vulkan implementations
target_include_directories(imgui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../glad/include
)

target_compile_options(imgui
    PRIVATE
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-O3>
)