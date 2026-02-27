# Dodo Engine

**Dodo** is a multi-platform game engine written in **C++** using **OpenGL**.

The engine features an **Entity Component System (ECS)**, model loading, materials, shadows and an in-engine editor.

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

## Screenshots

### Sponza Scene

![Sponza Scene](https://i.imgur.com/feKrrhv.png)

### Editor
![Level editor](https://i.imgur.com/6pS0l5n.png)


---

### Requirements
- CMake $\geq 3.25$ (although probably works with older versions)
- Ninja
- A C++20-compatible compiler
- OpenGL 3.3-capable GPU

### Cloning

This project uses **git submodules**. Make sure to clone with:

```bash
git clone --recurse-submodules git@github.com:quesswho/Dodo.git