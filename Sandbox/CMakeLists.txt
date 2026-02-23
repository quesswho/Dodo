add_executable(Sandbox)

target_sources(Sandbox
    PRIVATE
        src/**/*.cpp
        src/**/*.h
)

target_link_libraries(Sandbox PRIVATE Dodo)

target_compile_definitions(Sandbox
    PRIVATE
        _CRT_SECURE_NO_WARNINGS
        $<$<CONFIG:Debug>:DD_DEBUG>
        $<$<CONFIG:Release>:DD_RELEASE>
)

if (WIN32)
    target_compile_definitions(Sandbox PRIVATE DD_x64 DD_API_WIN32)
endif()