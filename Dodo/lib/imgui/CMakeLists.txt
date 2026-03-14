add_library(imgui STATIC)

target_sources(imgui PRIVATE
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/misc/cpp/imgui_stdlib.cpp
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
    target_link_libraries(imgui PRIVATE glad)
endif()

if(DD_API_VULKAN)
    target_sources(imgui PRIVATE
        imgui/backends/imgui_impl_vulkan.cpp
    )
    target_link_libraries(imgui PRIVATE glad)
    #target_compile_definitions(imgui PRIVATE IMGUI_IMPL_VULKAN_USE_LOADER)
endif()

if(DD_API_GLFW)
	target_sources(imgui PRIVATE
        imgui/backends/imgui_impl_glfw.cpp
    )
    target_link_libraries(imgui PUBLIC glfw)
elseif(DD_API_WIN32)
    target_sources(imgui PRIVATE
        imgui/backends/imgui_impl_win32.cpp
    )
endif()

target_compile_options(imgui
    PRIVATE
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-O3>
)