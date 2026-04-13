# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure (Ninja Multi-Config, recommended)
cmake --preset default

# Build
cmake --build --preset debug
cmake --build --preset release

# Switch backends (default is OpenGL)
cmake --preset default -DDD_API_OPENGL=OFF -DDD_API_VULKAN=ON

# Enable optional targets
cmake --preset default -DCOMPILE_EDITOR=ON -DCOMPILE_GAME=ON

# Binaries land in build/bin/{Debug|Release}/
```

Key CMake options: `DD_API_OPENGL`, `DD_API_VULKAN`, `DD_API_GLFW`, `DD_API_WIN32`, `COMPILE_SANDBOX`, `COMPILE_EDITOR`, `COMPILE_GAME`.

Slang SDK (2026.5.2) is auto-downloaded from GitHub at configure time; no manual install needed.

## Architecture Overview

**Dodo** is a multi-backend graphics engine. The core abstraction is compile-time: `RenderAPI`, `Pipeline`, `Buffer`, `Texture`, `CubeMap`, and `FrameBuffer` are typedefs that resolve to the active backend (OpenGL or Vulkan) via preprocessor macros (`DD_OPENGL` / `DD_VULKAN`).

```
Dodo/src/
├── Core/
│   ├── Application/   - Window, Event, Layer, Input
│   ├── Graphics/      - Renderer3D, Material, Pipeline, Scene
│   ├── ECS/           - Templated World<ComponentTypes...> + ComponentPool<T>
│   ├── Data/          - AssetManager (async/threaded), Model/Texture/Shader loaders
│   └── Shaders/       - .slang source files
└── Platform/
    ├── GraphicAPI/OpenGL/   - OpenGL 4.5 backend
    ├── GraphicAPI/Vulkan/   - Vulkan 1.3 backend (active development)
    └── WindowAPI/GLFW|Win32/
```

Applications (`Sandbox/`, `DodoEditor/`, `Game/`) sit outside the library and link against it.

## Shader Pipeline

All shaders are written in **Slang** (`.slang`), located in `res/shader/builtin/`. The compilation flow:

1. `SlangCompiler` compiles `.slang` → SPIR-V
2. For OpenGL: SPIRV-Cross translates SPIR-V → GLSL at runtime
3. `ShaderAsset` stores both representations plus reflection data (descriptor bindings, vertex input locations, push constant layout)

Shader modules are composed via Slang `#include`. Key shared modules live in `res/shader/builtin/Common/` (uniforms, vertex I/O structs, material helpers, lighting).

## Backend Differences

| | OpenGL | Vulkan |
|---|---|---|
| Push constants | UBO emulation | Native `vk::push_constant` / Slang args |
| Descriptors | Texture units + UBOs | Descriptor sets from reflection |
| Memory | Driver-managed | VMA |
| Sampler binding | Per-slot | Combined in descriptor set |

The Vulkan backend is under active development on `feature/vulkan`. OpenGL is the stable baseline.

## Key Patterns

- **Factory via RenderAPI**: All GPU resources (buffers, textures, samplers, pipelines, framebuffers) are created through `RenderAPI` factory methods, never constructed directly.
- **Material = Pipeline + Textures + Sampler**: A `Material` binds a compiled `Pipeline` to texture slots; `TextureSampler` is separate to match Vulkan's descriptor model.
- **Async asset loading**: `AssetManager` uses a thread pool; loading Sponza-scale scenes is expected to be async. Don't assume assets are ready synchronously.
- **Math conventions**: Column-major layout (`DD_MATH_COLUMN_MAJOR`), right-handed coordinate system (`DD_COORDINATE_RIGHT_HANDED`) — set at compile time.
- **Precompiled header**: `src/pch.h` is included project-wide; add heavy or frequently used headers there.
- **ImGui**: Frame setup/teardown is managed by `RenderAPI`, not the application layer.
