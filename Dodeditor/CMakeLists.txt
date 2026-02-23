add_executable(Dodeditor)

target_sources(Dodeditor
    PRIVATE
        src/**/*.cpp
        src/**/*.h
        res/**/*
)

target_link_libraries(Dodeditor PRIVATE Dodo)

target_compile_definitions(Dodeditor
    PRIVATE
        _CRT_SECURE_NO_WARNINGS
        $<$<CONFIG:Debug>:DD_DEBUG>
        $<$<CONFIG:Release>:DD_RELEASE>
)

if (WIN32)
    target_compile_definitions(Dodeditor PRIVATE DD_x64 DD_API_WIN32)
endif()