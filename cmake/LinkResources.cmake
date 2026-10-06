# Links the res/ directory into the runtime output directory.
#
# Every application shares the same output directory, so the link is created by a single
# target (DodoResources) rather than by a post-build step on each executable. Per-executable
# steps race against each other when the applications are linked in parallel.

get_property(_DD_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_DD_MULTI_CONFIG)
    set(_DD_RES_LINK_PARENT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>")
else()
    set(_DD_RES_LINK_PARENT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

if(WIN32)
    add_custom_target(DodoResources
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_DD_RES_LINK_PARENT}"
        COMMAND ${CMAKE_COMMAND}
            "-DLINK_DIR=${_DD_RES_LINK_PARENT}/res"
            "-DTARGET_DIR=${CMAKE_SOURCE_DIR}/res"
            -P "${CMAKE_SOURCE_DIR}/cmake/CreateJunction.cmake"
        COMMENT "Linking res/ into output directory"
        VERBATIM
    )
else()
    add_custom_target(DodoResources
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_DD_RES_LINK_PARENT}"
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${_DD_RES_LINK_PARENT}/res"
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            "${CMAKE_SOURCE_DIR}/res"
            "${_DD_RES_LINK_PARENT}/res"
        COMMENT "Linking res/ into output directory"
        VERBATIM
    )
endif()
