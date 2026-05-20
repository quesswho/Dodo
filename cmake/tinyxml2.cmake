message(STATUS "=== Configuring tinyxml2 ===")

include(FetchContent)

FetchContent_Declare(
    tinyxml2
    URL https://github.com/leethomason/tinyxml2/archive/refs/tags/10.0.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(tinyxml2_BUILD_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(tinyxml2)
