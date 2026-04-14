add_executable(Game)

if(WIN32)
    add_custom_command(TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            "-DLINK_DIR=$<TARGET_FILE_DIR:Game>/res"
            "-DTARGET_DIR=${CMAKE_SOURCE_DIR}/res"
            -P "${CMAKE_SOURCE_DIR}/cmake/CreateJunction.cmake"
        COMMENT "Linking res/ into output directory"
    )
else()
    add_custom_command(TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:Game>/res"
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            "${CMAKE_SOURCE_DIR}/res"
            "$<TARGET_FILE_DIR:Game>/res"
        COMMENT "Linking res/ into output directory"
    )
endif()


file(GLOB_RECURSE DODO_CPP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

file(GLOB_RECURSE DODO_H_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

target_sources(Game PRIVATE
    ${DODO_CPP_SOURCES}
    ${DODO_H_SOURCES}
)


target_link_libraries(Game PRIVATE Dodo)

target_compile_definitions(Game PRIVATE
        _CRT_SECURE_NO_WARNINGS
        $<$<CONFIG:Debug>:DD_DEBUG>
        $<$<CONFIG:Release>:DD_RELEASE>
)

if (DD_API_WIN32)
    target_compile_definitions(Game PRIVATE DD_x64 DD_API_WIN32)
endif()