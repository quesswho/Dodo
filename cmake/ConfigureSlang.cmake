message(STATUS "=== Configuring Slang ${SLANG_VERSION} ===")

set(SLANG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dodo/lib/slang")
set(SLANG_GENERATORS_BUILD_DIR "${CMAKE_BINARY_DIR}/slang-generators-build")

set(SLANG_GENERATORS_PATH "${SLANG_GENERATORS_INSTALL_DIR}/bin" CACHE PATH "" FORCE)
set(SLANG_LIB_TYPE "STATIC")
set(SLANG_ENABLE_GFX OFF)
set(SLANG_ENABLE_SLANG_RHI OFF)
set(SLANG_ENABLE_TESTS OFF)
set(SLANG_ENABLE_SLANGD OFF)
set(SLANG_ENABLE_REPLAYER OFF)
set(SLANG_SLANG_LLVM_FLAVOR "DISABLE")
set(SLANG_ENABLE_EXAMPLES OFF)
set(SLANG_ENABLE_XLIB OFF)
set(SLANG_ENABLE_CUDA OFF)
set(SLANG_ENABLE_OPTIX OFF)
set(SLANG_ENABLE_NVAPI OFF)
set(SLANG_ENABLE_AFTERMATH OFF)

# Check for slang-fiddle specifically since it's the last generator added
if(NOT EXISTS "${SLANG_GENERATORS_INSTALL_DIR}/bin/slang-fiddle.exe" AND
   NOT EXISTS "${SLANG_GENERATORS_INSTALL_DIR}/bin/slang-fiddle")

    message(STATUS "Building Slang generators")

    # Configure the generator
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -S "${SLANG_SOURCE_DIR}"
            -B "${SLANG_GENERATORS_BUILD_DIR}"
            -G "${CMAKE_GENERATOR}"
            --preset default
    )

    message(STATUS "Slang generators installed to ${SLANG_GENERATORS_INSTALL_DIR}/bin")
endif()