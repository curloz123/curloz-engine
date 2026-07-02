# Curloz Engine

A Vulkan 1.3 game engine written in C++23. 


## Tech Stack

| System | Library        |
|---|----------------|
| Renderer | Vulkan 1.3     |
| Windowing | GLFW           |
| ECS | clz::ecs#      |
| Physics | Jolt Physics*  |
| Audio | OpenAL Soft*   |
| Math | clz::math#     |
| Model Loading | assimp         |
| Animation | ozz-animation* |
| Build System | CMake + Ninja  |

---
> '#' Signifies that no external library is used and is self authorized

> '*' Signifies that system is planned and has not yet been implemented yet, or needs further work
---

## Prerequisites

### Linux
- Vulkan 1.3 capable GPU and drivers
- CMake 3.25+
- Ninja
- clang-format (for code style enforcement)
- GCC or Clang with C++23 support

On Gentoo:
```bash
sudo emerge -av cmake ninja clang dev-util/vulkan-tools
```

On Ubuntu/Debian:
```bash
sudo apt install cmake ninja-build clang-format vulkan-tools libvulkan-dev
```

### Windows
- CMake 3.25+
- Ninja
- Vulkan SDK from [lunarg.com](https://vulkan.lunarg.com)
- MSVC or MinGW with C++23 support

---

## Building

```bash
# Clone with submodules
git clone --recursive https://github.com/curl0z/curloz-engine.git
cd curloz-engine

# If you forgot --recursive
git submodule update --init --recursive

# Configure and build
cmake -B build/debug -G Ninja
cmake --build build/debug
```

Binary lands at `build/debug/CurlozEngine`.

---

## Project Structure

```
curloz-engine/
├── src/                # Source files (.cpp)
├── include/            # Header files (.hpp)
├── shaders/            # GLSL shaders, compiled to .spv
├── assets/             # Models, textures, audio
├── external/           # Git submodules (do not modify manually)
├── core/               # Utility functions, basically helpers used in entire engine
├── .clang-format	# clang-format configuration
├── .editorconfig	# editorconfig configuration
├── CMakeLists.txt	# CMake build configuration
└── LICENSE		# License file
```

---

## Want to Contribute?

Contributors are always welcome! If you feel there is any issue, or think there's something that can either be added or be better, feel free to open an issue or submit a pull request.

If you think can handle a subsystem for long term, feel free to approach and it will be assigned to you.

### To-Do
- Display model component's Node->Mesh->Primitives hierarchy in editor's inspector

For More details, see the [CONTRIBUTING.md](CONTRIBUTING.md) file.

## License

MIT
