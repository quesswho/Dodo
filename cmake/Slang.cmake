message(STATUS "=== Configuring Slang ===")

set(SLANG_VERSION "2026.5.2" CACHE STRING "Slang version")

# Detect os and arch
if(WIN32)
    set(SLANG_OS "windows")
    set(SLANG_ARCH "x86_64")
    set(SLANG_ARCHIVE_EXT "zip")
else()
    set(SLANG_OS "linux")
    set(SLANG_ARCH "x86_64")
    set(SLANG_ARCHIVE_EXT "zip")
endif()

include(FetchContent)

set(SLANG_DOWNLOAD_URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-${SLANG_OS}-${SLANG_ARCH}.zip)
message(STATUS "Downloading Slang from ${SLANG_DOWNLOAD_URL}")

set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(
    slang_sdk
    URL "${SLANG_DOWNLOAD_URL}"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(slang_sdk)

# slang_sdk_SOURCE_DIR is set automatically by FetchContent
set(SLANG_INSTALL_DIR "${slang_sdk_SOURCE_DIR}" CACHE PATH "" FORCE)

find_package(slang REQUIRED
    PATHS ${SLANG_INSTALL_DIR}
    NO_DEFAULT_PATH
)

# Collect runtime files
if(WIN32)
    set(SLANG_RUNTIME_FILES
        "${SLANG_INSTALL_DIR}/bin/slang-compiler.dll"
        "${SLANG_INSTALL_DIR}/bin/slang-glsl-module.dll"
        "${SLANG_INSTALL_DIR}/bin/slang-glslang.dll"
    )
else()
    file(GLOB SLANG_RUNTIME_FILES
        "${SLANG_INSTALL_DIR}/lib/libslang-compiler.so*"
        "${SLANG_INSTALL_DIR}/lib/libslang-glsl-module*.so"
        "${SLANG_INSTALL_DIR}/lib/libslang-glslang*.so"
    )
endif()

message(STATUS "Slang ${SLANG_VERSION} configured from ${SLANG_INSTALL_DIR}")