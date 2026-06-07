# Dodo Engine
[![CMake on multiple platforms](https://github.com/quesswho/Dodo/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/quesswho/Dodo/actions/workflows/cmake-multi-platform.yml)

**Dodo** is a multi-platform game engine written in **C++**, built from scratch.
It features a graphics API abstraction layer, but currently only supports **Vulkan**

The engine features an Entity Component System (ECS), model loading, PBR materials, shadow maps and a level editor.

---

### Sponza Scene

![Sponza Scene](https://i.imgur.com/feKrrhv.png)

### Editor
![Level editor](https://i.imgur.com/6pS0l5n.png)

### Voxel Game
![Voxel Game](https://i.imgur.com/VqE9Zm5.png)

---

## Project Structure

- `Dodo/` - Engine source code
- `Sandbox/` - Example application
- `DodoEditor/` - Editor code
- `Game/` - Procedurally generated voxel game
- `res/` - Engine and sample assets
- `build/` - Generated build files

## Libraries Used
- **GLFW** - Window creation & input (optional, cross-platform)
- **Slang** - Intermediate shading language compiler
- **Volk** - Vulkan loader (only Vulkan)
- **VMA** - Vulkan Memory Allocator (only Vulkan)
- **ImGui** - GUI for the level editor
- **spdlog** - High-performance logging
- **stb_image** - Texture loading
- **Assimp** - Model loading (`.fbx`, `.obj`, etc.)

---

## Compilation

### Requirements
- CMake >= 3.26
- Ninja
- A C++20-compatible compiler
- Vulkan 1.3-capable GPU (for Vulkan backend)

### Cloning

This project uses **git submodules**. Make sure to clone with:

```bash
git clone --recurse-submodules git@github.com:quesswho/Dodo.git
git submodule update --init --recursive
```

### Building and running
To configure Dodo run

```
cmake --preset default
```

To build and run:
```
cmake --build --preset debug
make run
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `DD_API_VULKAN` | `ON` | Enable Vulkan backend |
| `DD_API_WIN32` | `OFF` | Enable Win32 window backend |
| `DD_API_GLFW` | `ON` | Enable GLFW window backend |
| `COMPILE_EDITOR` | `ON` | Compile the level editor |
| `COMPILE_SANDBOX` | `ON` | Compile the sandbox project |
| `COMPILE_GAME` | `ON` | Compile the voxel game |


### Graphics
- PBR: Cook-Torrance BRDF
![Cook-Torrance BRDF](https://i.imgur.com/qfpEHyE.png)
- We use cascaded shadow maps for directional lights:
![Cascaded Shadow Maps](https://i.imgur.com/fBgjl41.png)
- Percentage-Closer Soft Shadows using Poisson disc sampling
![PCSS and Poisson disc sampling](https://i.imgur.com/aIfWOVk.png)
- Alpha test in shadow pass through bindless textures
![Alpha test](https://i.imgur.com/5idSnW7.png)