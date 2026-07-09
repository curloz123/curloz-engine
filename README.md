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
git clone --recursive https://github.com/curloz123/curloz-engine.git
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

### Git submodules are missing or not initialized

The project uses Git submodules for third-party dependencies stored under the `external/` directory. If CMake reports missing source directories, missing `CMakeLists.txt` files, or errors while adding external libraries, verify that all submodules have been initialized.

Check the current submodule status:

```bash
git submodule status
```

If a submodule has not been initialized, its status may be prefixed with `-`.

Initialize and update all submodules recursively:

```bash
git submodule update --init --recursive
```

If the repository was cloned without submodules, the same command can be run from the repository root after cloning.

Alternatively, clone the repository and initialize submodules in one step:

```bash
git clone --recursive https://github.com/curloz123/curloz-engine.git
cd curloz-engine
```

If submodule URLs or references appear out of sync, synchronize them before updating:

```bash
git submodule sync --recursive
git submodule update --init --recursive
```

After restoring the submodules, remove the previous build directory and configure the project again:

```bash
cmake -E remove_directory build/debug
cmake -B build/debug -G Ninja
cmake --build build/debug
```

### Compiler does not support the required C++ standard

Curloz Engine is configured to use C++23. Configuration or compilation may fail when the selected compiler does not support the required language features or when CMake selects a different compiler than expected.

Check the active compiler version before configuring the project.

For GCC:

```bash
g++ --version
```

For Clang:

```bash
clang++ --version
```

For MSVC, open a Visual Studio Developer Command Prompt and run:

```powershell
cl
```

If multiple compilers are installed, CMake may select a compiler different from the one you intended to use.

To explicitly configure with GCC:

```bash
cmake -B build/debug -G Ninja \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++
```

To explicitly configure with Clang:

```bash
cmake -B build/debug -G Ninja \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++
```

When switching compilers, do not reuse an existing configured build directory. Remove it before running CMake again:

```bash
cmake -E remove_directory build/debug
cmake -B build/debug -G Ninja
cmake --build build/debug
```

If compilation still fails, verify that the selected compiler has suitable C++23 support and that CMake is detecting the intended compiler during configuration.

---

## Project Structure

```text
curloz-engine/
├── src/                # Source files (.cpp)
├── include/            # Header files (.hpp)
├── shaders/            # GLSL shaders, compiled to .spv
├── assets/             # Models, textures, audio
├── external/           # Git submodules (do not modify manually)
├── core/               # Utility functions, basically helpers used in entire engine
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
