add_executable(DodoEditor)

file(GLOB_RECURSE DODOEDITOR_CPP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

file(GLOB_RECURSE DODOEDITOR_H_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

file(GLOB_RECURSE DODOEDITOR_RES
    ${CMAKE_CURRENT_SOURCE_DIR}/res/**/*
)

target_sources(DodoEditor
    PRIVATE
        ${DODOEDITOR_CPP_SOURCES}
        ${DODOEDITOR_H_SOURCES}
)

target_link_libraries(DodoEditor PRIVATE Dodo)

target_compile_definitions(DodoEditor
    PRIVATE
        _CRT_SECURE_NO_WARNINGS
        $<$<CONFIG:Debug>:DD_DEBUG>
        $<$<CONFIG:Release>:DD_RELEASE>
)

if (DD_API_WIN32)
    target_compile_definitions(DodoEditor PRIVATE DD_x64 DD_API_WIN32)
endif()