# Dodo Engine
[![CMake on multiple platforms](https://github.com/quesswho/Dodo/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/quesswho/Dodo/actions/workflows/cmake-multi-platform.yml)

**Dodo** is a multi-platform game engine written in **C++**, built from scratch.
It features a graphics API abstraction layer with an **OpenGL** backend and a **Vulkan** backend in progress.

The engine features an Entity Component System (ECS), model loading, materials, shadow maps, and a level editor.

---

### Sponza Scene

![Sponza Scene](https://i.imgur.com/feKrrhv.png)

### Editor
![Level editor](https://i.imgur.com/6pS0l5n.png)

### Voxel Game
![Voxel Game](https://i.imgur.com/eSYjuAw.png)

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
- **Volk** - Vulkan loader
- **GLAD** - OpenGL function loader
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
- OpenGL 4.5-capable GPU
- Vulkan 1.3-capable GPU (for Vulkan backend, optional)

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
| `DD_API_OPENGL` | `ON` | Enable OpenGL backend |
| `DD_API_VULKAN` | `OFF` | Enable Vulkan backend |
| `DD_API_WIN32` | `OFF` | Enable Win32 window backend |
| `DD_API_GLFW` | `ON` | Enable GLFW window backend |
| `COMPILE_EDITOR` | `ON` | Compile the level editor |
| `COMPILE_SANDBOX` | `OFF` | Compile the sandbox project |
| `COMPILE_GAME` | `OFF` | Compile the voxel game |

Example: build with Vulkan instead of OpenGL:
```
cmake --preset default -DDD_API_OPENGL=OFF -DDD_API_VULKAN=ON
```