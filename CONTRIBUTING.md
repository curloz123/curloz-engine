# Maintainer Guide

This document is for contributors working on Curloz Engine. Read this before writing a single line.

## General Architecture

Curloz Engine is built as a collection of independent subsystems under the clz:: namespace, each following the same lifecycle pattern: an init() to set up state, an update() that runs once per frame, and a shutdown() that tears everything down in reverse order of initialization. Ofcourse not all subsystems require special init and destruction.

```bash
curloz-engine/
├── Renderer		# Renderer subsystem
├── Window		     # Window subsystem
├── Math			# Math subsystem
└── ECS			    # ECS subsystem
```

The init pattern is a linear sequence of init calls, with each subsystem initializing in dependency order. For example, the renderer subsystem depends on the window subsystem, so the window subsystem's init must be called before the renderer subsystem's init. Same is the case with shutdown.

Every subsystem shall stay decoupled from everything else, on purpose. We don't need to know about other subsystems to do our job. We just need to know how to interface with them, and get relevant data from them. For example, the renderer subsystem doesn't need to know about the ECS subsystem to render entities, but needs data about entities render component.

The main loop in main.cpp is the only place that knows about every subsystem at once, calling each one's init in dependency order at startup, update every frame, and shutdown in reverse order at the end.

---

## Subsystem Lifecycle

Curloz Engine is organized as a set of independent subsystems. Each subsystem is responsible for a specific part of the engine and is coordinated from `src/main.cpp`.

The engine lifecycle has three main phases:

1. Initialization
2. Per-frame update
3. Shutdown

Not every subsystem requires every lifecycle function. Some systems only perform initialization or shutdown work, while others participate in the main loop and update every frame.

### Lifecycle Overview

```text
Initialization
──────────────

Config
  ↓
Time
  ↓
Window
  ↓
Renderer
  ↓
Audio
  ↓
Scene Load


Main Loop
─────────

Compute Time
  ↓
Window Update
  ↓
Renderer Update
  ↓
Repeat until EngineState::Shutdown


Shutdown
────────

Scene Save
  ↓
Audio Shutdown
  ↓
Renderer Shutdown
  ↓
Window Shutdown
```

### Initialization Order

Subsystems are initialized in dependency order.

The current startup flow in `src/main.cpp` is:

```text
Config → Time → Window → Renderer → Audio → Scene
```

The order matters because later systems may depend on state created by earlier systems.

For example:

- Configuration is initialized first because other parts of the engine read application configuration.
- Time is initialized before entering the main loop.
- The window is initialized before the renderer because rendering requires a valid windowing environment.
- Audio is initialized after the core window and rendering systems are available.
- Scene data is loaded after the core runtime systems are available.

When adding a subsystem, determine its dependencies before choosing where its initialization belongs.

### Per-Frame Update Flow

After initialization and scene loading complete, the engine enters the main loop.

The current update order is:

```text
Time → Window → Renderer
```

The current main loop performs:

```cpp
while (clz::state::g_engineState != clz::state::EngineState::Shutdown)
{
	clz::time::computeTime();
	clz::window::update();
	clz::renderer::update();
}
```

The loop continues until the global engine state changes to `EngineState::Shutdown`.

Not every subsystem needs a per-frame update. A subsystem should only participate in the main loop when it has work that must be performed every frame.

### Shutdown Order

The current shutdown flow is:

```text
Scene Save → Audio → Renderer → Window
```

Scene state is saved before runtime systems are destroyed. Audio is shut down before the renderer and window, and the window is destroyed after the renderer has finished using it.

When adding shutdown logic, consider dependencies carefully. A subsystem should not attempt to use another subsystem that has already been destroyed.

### Subsystem Boundaries

Subsystems should remain independent and communicate through clearly defined interfaces.

A subsystem should:

- own the state related to its responsibility;
- expose only the operations and data required by other systems;
- avoid directly managing another subsystem's internal state;
- avoid unnecessary dependencies on unrelated systems;
- be initialized, updated, and shut down from the central engine flow when lifecycle coordination is required.

For example, the renderer should not become responsible for ECS behavior. It may consume the render data required to draw entities, but entity-management responsibilities should remain outside the renderer.

This separation keeps subsystems easier to understand, test, modify, and replace without creating unnecessary coupling.

### Adding a New Subsystem

Before adding a new subsystem to the engine lifecycle:

1. Define what the subsystem owns.
2. Identify which existing systems it depends on.
3. Place initialization after its required dependencies.
4. Add a per-frame update only when continuous processing is necessary.
5. Place shutdown logic so dependent resources are released safely.
6. Keep subsystem-specific behavior inside the subsystem instead of adding unrelated logic to `main.cpp`.

`main.cpp` should coordinate subsystem lifecycle order. It should not contain the internal implementation details of each subsystem.

## Who maintaining what

| Subsystem | Owner      | Scope |
|---|------------|---|
| Window | @curl0z    | Everything under **src or include/window/** |
| Renderer | @curl0z    | Everything under **src or include/renderer/** |
| Math | @curl0z | Everything under **include/math/** |
| Website | @harleen05 | index.html |
| Documentation | @harleen05 | index.html |
| Physics       | @curl0z    | Everything under **src or include/physics/**  |
| Audio | unassigned | |

**Do not touch code outside your area without discussing it first.** If you need something from another area, ask. Don't patch it yourself.

---

## Dev Environment Setup

### Linux

```bash
# Clone with submodules
git clone --recursive https://github.com/curl0z/curloz-engine.git
cd curloz-engine

# Install deps (Gentoo)
sudo emerge -av cmake ninja clang vulkan-tools

# Install deps (Ubuntu/Debian)
sudo apt install cmake ninja-build clang-format libvulkan-dev vulkan-tools

# Build
cmake -B build -G Ninja
cmake --build build
```

### Windows

- Install [Git for Windows](https://git-scm.com/download/win) (includes Git Bash — required)
- Install [CMake](https://cmake.org/download/)
- Install [Ninja](https://ninja-build.org/)
- Install [Vulkan SDK](https://vulkan.lunarg.com/)
- Open the repo folder in Visual Studio via **File → Open → Folder** (not Open Project)
- VS detects `CMakeLists.txt` automatically Don't use `.vcxproj` files

---

## Git Workflow

### Branches

```
main                  → always stable, always builds
feature/xxx           → new features
fix/xxx               → bug fixes
```

Never commit directly to `main`. Always branch, then PR.

```bash
# Start work
git checkout -b feature/physics-rigidbody

# Push branch
git push --set-upstream origin feature/physics-rigidbody

# Delete after merge
git branch -d feature/physics-rigidbody
git push origin --delete feature/physics-rigidbody
```

### Commit Messages

Format: `type: short description`

```
feature: added xyz feature
fix: fixed xyz bug
refactor: cleaned the messy code at xyz system/file
```

Types: `feature`, `fix`, `refactor`

No "fixed stuff". No "wip" on main. No giant commits that touch everything.

### Example Commit Messages

```text
feature: add physics subsystem initialization

fix: correct renderer initialization bug

docs: add practical contributing examples to CONTRIBUTING.md

refactor: simplify subsystem lifecycle documentation
```

### PRs

- Every branch merge goes through a PR, even between friends
- Write what you did and why in the PR description
- At least one review before merge

### Example Pull Request

**Title**

```text
docs: add practical contributing examples to CONTRIBUTING.md
```

**Description**

```md
## Summary

- Added sample commit message examples
- Added sample pull request description template
- Documentation only, no code changes

Fixes #28
```

---

## Code Style

- **Indentation** -> tabs, 8-wide
- **Braces** -> Allman everywhere (brace always on its own line)
- **Pointers** -> left aligned (`int* ptr`)
- **Functions** —> `camelCase`
- **Structs/Classes** —> `PascalCase`
- **Members/Variables** —> `camelCase`
- **Sub system variables** —> every variable described at sub system level must be prefixed as per the subsystem's name. For example in physics, variables must be prefixed with `ph_` etc..

Enforced by `.clang-format` and `.editorconfig` in the repo root.

### Setup (Linux)

```bash
# Install clang-format
sudo emerge -av clang  # Gentoo
sudo apt install clang-format  # Ubuntu

# Set up pre-commit hook (one time)
cp scripts/pre-commit .git/hooks/pre-commit
chmod +x .git/hooks/pre-commit
```

### Setup (Windows)

clang-format is built into Visual Studio. Enable it:

Tools → Options → Text Editor → C/C++ → Code Style → Formatting → **Enable ClangFormat support** → check **Format on Save**

VS picks up `.clang-format` from the repo root automatically.

### Fix Formatting Manually

```bash
find src include -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

---

## Doxygen Comment Standards

Every file, struct, and function must be documented. Use `///` style comments.

### File Header

Every `.cpp` and `.hpp` file starts with:

```cpp
/**
 * @file PhysicsSystem.hpp
 * @author Your Name
 * @brief Brief one line description of what this file contains
 */
```

mentioning the author and a little brief about what it does.

### Structs and Classes

```cpp
/**
 * @brief Stores position, rotation and scale of an entity in world space.
 *
 * This is the handshake between the ECS and physics systems.
 * Do not modify the layout without consulting @curl0z.
 */
struct TransformComponent
{
        glm::vec3 position; ///< World space position
        glm::quat rotation; ///< Rotation as quaternion
        glm::vec3 scale;    ///< Non-uniform scale
};
```

### Functions

```cpp
/**
 * @brief Submits a mesh for rendering this frame.
 *
 * @param data Mesh geometry and material data to submit
 * @return True if submission succeeded, false if render queue is full
 */
bool submitMesh(const MeshSubmitData& data);
```

### Enum

```cpp
/**
 * @brief Describes the current state of the physics body.
 */
enum class BodyState
{
        Active,   ///< Body is being simulated
        Sleeping, ///< Body has come to rest
        Static    ///< Body never moves
};
```

### Quick Reference

| Tag        | Use                         |
| ---------- | --------------------------- |
| `@file`    | File name                   |
| `@author`  | Who wrote it                |
| `@brief`   | One line summary            |
| `@param`   | Function parameter          |
| `@return`  | Return value                |
| `@note`    | Extra info                  |
| `@warning` | Something that can go wrong |
| `@todo`    | Planned work                |

---

## Doxygen Docs

Docs are **not committed to the repo**. If you want to view the docs, you need to generate them locally via doxxygen.

To generate locally:

```bash
doxygen Doxyfile
```

# Build Troubleshooting

If configuration or compilation fails, check the following common setup issues before opening a new issue.

## Vulkan SDK is not detected

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
cmake -E remove_directory build/linux/"Linux EngineDebug"
cmake --preset "Linux EngineDebug"
```

(Substitute the preset name and path for whichever one you're using — see [CMakePresets.json](../CMakePresets.json) for the full list.)

## CMake or Ninja is not available

Verify that the required build tools are installed and accessible from the command line:

```bash
cmake --version
ninja --version
```

The project requires CMake 3.25 or newer. If either command is not found, install the missing tool and reopen the terminal before configuring the project again.

## CMake generator mismatch

The project's [CMakePresets.json](../CMakePresets.json) pins the correct generator per platform (Ninja on Linux, Visual Studio on Windows), so this shouldn't come up if you configure via `cmake --preset ...`. It typically only happens if you mix a manual `cmake -B ... -G ...` invocation with a preset-configured build directory, or switch presets while reusing an old build directory.

A CMake build directory remembers the generator used when it was first configured. Reusing the same directory with a different generator produces a generator mismatch error.

Remove the existing build directory before configuring again with the intended preset:

```bash
cmake -E remove_directory build/linux/"Linux EngineDebug"
cmake --preset "Linux EngineDebug"
cmake --build --preset "Linux EngineDebug"
```

## Git submodules are missing or not initialized

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
cmake -E remove_directory build/linux/"Linux EngineDebug"
cmake --preset "Linux EngineDebug"
cmake --build --preset "Linux EngineDebug"
```

## Compiler does not support the required C++ standard

Curloz Engine is configured to use C++23. Configuration or compilation may fail when the selected compiler does not support the required language features or when CMake selects a different compiler than expected.

For a stable build, use one of the following minimum compiler versions:

- GCC 13 or newer
- Clang 17 or newer
- MSVC 19.38 or newer, included with Visual Studio 2022 version 17.8 or newer

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

If multiple compilers are installed, CMake may select a compiler different from the one you intended to use. You can override the compiler on top of a preset with `-D` flags:

To explicitly configure with GCC:

```bash
cmake --preset "Linux EngineDebug" \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++
```

To explicitly configure with Clang:

```bash
cmake --preset "Linux EngineDebug" \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++
```

When switching compilers, do not reuse an existing configured build directory. Remove it before running CMake again:

```bash
cmake -E remove_directory build/linux/"Linux EngineDebug"
cmake --preset "Linux EngineDebug" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build --preset "Linux EngineDebug"
```

If compilation still fails, verify that the selected compiler has suitable C++23 support and that CMake is detecting the intended compiler during configuration.

Output goes to `docs/` which is in `.gitignore`. Do not commit it.
