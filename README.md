# Curloz Engine

A Vulkan 1.3 game engine written in C++23.

## Tech Stack

| System        | Library        |
| ------------- | -------------- |
| Renderer      | Vulkan 1.3     |
| Windowing     | GLFW           |
| ECS           | clz::ecs#      |
| Physics       | Jolt Physics*  |
| Audio         | OpenAL Soft*   |
| Math          | clz::math#     |
| Model Loading | assimp         |
| Animation     | ozz-animation* |
| Build System  | CMake + Ninja  |

---

> '#' Signifies that no external library is used and is self authorized

> '*' Signifies that system is planned and has not yet been implemented yet, or needs further work

---

## Prerequisites

### Linux

* Vulkan 1.3 capable GPU and drivers
* CMake 3.25+
* Ninja
* clang-format (for code style enforcement)
* GCC or Clang with C++23 support

On Gentoo:

```bash
sudo emerge -av cmake ninja clang dev-util/vulkan-tools
```

On Ubuntu/Debian:

```bash
sudo apt install cmake ninja-build clang-format vulkan-tools libvulkan-dev
```

### Windows

* CMake 3.25+
* Ninja
* Vulkan SDK from [lunarg.com](https://vulkan.lunarg.com)
* MSVC or MinGW with C++23 support

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

## Build Troubleshooting

If configuration or compilation fails, check the following common setup issues before opening a new issue.

### Vulkan SDK is not detected

The project uses CMake's Vulkan package discovery during configuration. If CMake reports that Vulkan could not be found, first verify that the Vulkan SDK and tools are available on your system.

Check whether the Vulkan tools are accessible from the terminal:

```bash
vulkaninfo --summary
```

If the command is not found, verify that the Vulkan SDK is installed and that its tools are available through the system `PATH`.

On Windows, verify that the Vulkan SDK environment variable is set:

```powershell
echo $env:VULKAN_SDK
```

The command should print the path to the installed Vulkan SDK. If it is empty, verify the SDK installation and environment configuration, then restart the terminal before running CMake again.

After correcting the SDK setup, remove the previous build directory and configure the project again:

```bash
cmake -E remove_directory build/debug
cmake -B build/debug -G Ninja
```

### CMake or Ninja is not available

Verify that the required build tools are installed and accessible from the command line:

```bash
cmake --version
ninja --version
```

The project requires CMake 3.25 or newer. If either command is not found, install the missing tool and reopen the terminal before configuring the project again.

### CMake generator mismatch

The documented build commands use the Ninja generator:

```bash
cmake -B build/debug -G Ninja
cmake --build build/debug
```

A CMake build directory remembers the generator used when it was first configured. Reusing the same directory with a different generator may produce a generator mismatch error.

Remove the existing build directory before configuring it again with Ninja:

```bash
cmake -E remove_directory build/debug
cmake -B build/debug -G Ninja
cmake --build build/debug
```

---

## Project Structure

```text
curloz-engine/
├── src/                # Source files (.cpp)
├── include/            # Header files (.hpp)
│   └── core/           # Utility functions, basically helpers used in entire engine
├── shaders/            # GLSL shaders, compiled to .spv
├── assets/             # Models, textures, audio
├── config/             # Engine/scene configuration (engine.toml, scene.json)
├── external/           # Git submodules (do not modify manually)
├── .clang-format       # clang-format configuration
├── .editorconfig       # editorconfig configuration
├── CMakeLists.txt      # CMake build configuration
└── LICENSE             # License file
```

---

## Want to Contribute?

Contributors are always welcome! If you feel there is any issue, or think there's something that can either be added or be better, feel free to open an issue or submit a pull request.

If you think can handle a subsystem for long term, feel free to approach and it will be assigned to you.

### To-Do

* Display model component's Node->Mesh->Primitives hierarchy in editor's inspector

For More details, see the [CONTRIBUTING.md](CONTRIBUTING.md) file.

## License

MIT
