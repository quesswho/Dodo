# Dodo Engine

**Dodo** is a multi-platform game engine written in **C++** using **OpenGL**.

The engine features an **Entity Component System (ECS)**, model loading, materials, shadows and an editor.

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
- `res/` - Engine and sample assets
- `build/` - Generated build files

## Libraries Used

Dodo uses the following libraries:

- **Win32 / GLFW** – Window creation & input
- **GLAD** – OpenGL function loader
- **ImGui** – GUI for the level editor
- **spdlog** – High-performance logging
- **stb_image** – Texture loading
- **Assimp** – Model loading (`.fbx`, `.obj`, etc.)

---

## Compilation

### Requirements
- CMake >= 3.26
- Ninja
- A C++20-compatible compiler
- OpenGL 3.3-capable GPU

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
