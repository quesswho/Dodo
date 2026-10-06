add_executable(Sandbox)

set_target_properties(Sandbox PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Sandbox>"
)

add_dependencies(Sandbox DodoResources)

if(WIN32)
    add_custom_command(TARGET Sandbox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:Sandbox>
            $<TARGET_FILE_DIR:Sandbox>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime DLLs"
    )
endif()


file(GLOB_RECURSE DODO_CPP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

file(GLOB_RECURSE DODO_H_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

target_sources(Sandbox PRIVATE
    ${DODO_CPP_SOURCES}
    ${DODO_H_SOURCES}
)


target_link_libraries(Sandbox PRIVATE Dodo)

target_compile_definitions(Sandbox PRIVATE
        _CRT_SECURE_NO_WARNINGS
        $<$<CONFIG:Debug>:DD_DEBUG>
        $<$<CONFIG:Release>:DD_RELEASE>
)

target_compile_options(Sandbox PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/MP>
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
    $<$<CXX_COMPILER_ID:MSVC>:/EHsc>
)

if (DD_API_WIN32)
    target_compile_definitions(Sandbox PRIVATE DD_x64 DD_API_WIN32)
endif()