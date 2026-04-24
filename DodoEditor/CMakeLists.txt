add_executable(DodoEditor)

set_target_properties(DodoEditor PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:DodoEditor>"
)

if(WIN32)
    add_custom_command(TARGET DodoEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            "-DLINK_DIR=$<TARGET_FILE_DIR:DodoEditor>/res"
            "-DTARGET_DIR=${CMAKE_SOURCE_DIR}/res"
            -P "${CMAKE_SOURCE_DIR}/cmake/CreateJunction.cmake"
        COMMENT "Linking res/ into output directory"
    )
    add_custom_command(TARGET DodoEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:DodoEditor>
            $<TARGET_FILE_DIR:DodoEditor>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime DLLs"
    )
else()
    add_custom_command(TARGET DodoEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:DodoEditor>/res"
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            "${CMAKE_SOURCE_DIR}/res"
            "$<TARGET_FILE_DIR:DodoEditor>/res"
        COMMENT "Linking res/ into output directory"
    )
endif()


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
        lib/tinyfiledialogs/tinyfiledialogs.cpp
)

target_link_libraries(DodoEditor PRIVATE Dodo)

target_include_directories(DodoEditor
    PRIVATE
        src
        lib/tinyfiledialogs
)

target_compile_definitions(DodoEditor
    PRIVATE
        _CRT_SECURE_NO_WARNINGS
        $<$<CONFIG:Debug>:DD_DEBUG>
        $<$<CONFIG:Release>:DD_RELEASE>
)

target_compile_options(DodoEditor PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/MP>
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
    $<$<CXX_COMPILER_ID:MSVC>:/EHsc>
)

if (DD_API_WIN32)
    target_compile_definitions(DodoEditor PRIVATE DD_x64 DD_API_WIN32)
endif()