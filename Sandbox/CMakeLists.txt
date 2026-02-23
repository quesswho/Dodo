add_executable(Sandbox)

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

if (WIN32)
    target_compile_definitions(Sandbox PRIVATE DD_x64 DD_API_WIN32)
endif()