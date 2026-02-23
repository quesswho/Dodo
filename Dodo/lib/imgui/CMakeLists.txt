add_library(imgui STATIC)

target_sources(imgui PRIVATE
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
)

# We use glad for imgui opengl/vulkan implementations
target_include_directories(imgui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../glad/include
)

if(DD_API_OPENGL)
    target_sources(imgui PRIVATE
        imgui/backends/imgui_impl_opengl3.cpp
    )
endif()

if(DD_API_VULKAN)
    target_sources(imgui PRIVATE
        imgui/backends/imgui_impl_vulkan.cpp
    )
endif()

if(WIN32)
    target_sources(imgui PRIVATE
        imgui/backends/imgui_impl_win32.cpp
    )
endif()

target_compile_options(imgui
    PRIVATE
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-O3>
)