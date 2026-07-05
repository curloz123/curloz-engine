cmake_minimum_required(VERSION 3.25)
project(CurlozEngine VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


## Sources
file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE include)

## Find Packages Section
find_package(Vulkan REQUIRED)

## Compile options

# Compile Profile
set(CMAKE_CONFIGURATION_TYPES "Debug;Editor;RelWithDebInfo;Release" CACHE STRING "" FORCE)
target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:CLZ_DEBUG CLZ_ENABLE_SANDBOX CLZ_ENABLE_LOGS>
        $<$<CONFIG:Sandbox>:CLZ_ENABLE_SANDBOX CLZ_ENABLE_LOGS>
        $<$<CONFIG:RelWithDebInfo>:CLZ_RELEASE CLZ_ENABLE_LOGS>
        $<$<CONFIG:Release>:CLZ_RELEASE>
)
target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-Wall -Wextra -Wsign-compare>
)

#SSE 4.1 instruction sets
target_compile_options(${PROJECT_NAME} PRIVATE -msse4.1 -mfma)


## Always use static libs
set(BUILD_SHARED_LIBS OFF)

## External Libs, This should be the last section, everything below this is just linking third party libraries

#Vulkan
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

#GLFW
add_subdirectory(external/glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

#TOMLPLUSPLUS
add_subdirectory(external/tomlplusplus)
target_link_libraries(${PROJECT_NAME} PRIVATE tomlplusplus::tomlplusplus)

#OPENAL-SOFT
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/openal-soft)
target_compile_options(OpenAL PRIVATE -Wno-function-effects)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenAL)

#JSON
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
add_subdirectory(external/json)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

#STB_IMAGE
target_include_directories(${PROJECT_NAME} PRIVATE external/stb)

# ASSIMP
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/assimp)
target_compile_options(assimp PRIVATE -Wno-error -Wno-unsafe-buffer-usage)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)

#IMGUI
add_library(imgui STATIC
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/backends/imgui_impl_vulkan.cpp
)
target_include_directories(imgui PUBLIC
        external/imgui
        external/imgui/backends
)
target_link_libraries(imgui PUBLIC
        Vulkan::Vulkan
        glfw
)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)

#IMGUIZMO
add_library(imguizmo STATIC
        external/ImGuizmo/src/ImGuizmo.cpp
)
target_include_directories(imguizmo PUBLIC
        external/ImGuizmo/src
)
target_link_libraries(imguizmo PUBLIC imgui)
target_compile_options(imguizmo PRIVATE -Wno-error -Wno-unsafe-buffer-usage)
target_link_libraries(${PROJECT_NAME} PRIVATE imguizmo)

#BOX3D
add_subdirectory(external/box3d)
target_link_libraries(${PROJECT_NAME} PRIVATE box3d::box3d)