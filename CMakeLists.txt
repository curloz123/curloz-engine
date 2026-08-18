cmake_minimum_required(VERSION 3.25)

project(CurlozEngine
        VERSION 0.3.6
        DESCRIPTION "A 3D Game Engine"
        HOMEPAGE_URL "https://curlozengine.vercel.app/"
        LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

## Sources
file(GLOB_RECURSE SOURCES src/*.cpp editor/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE include editor)

## Find Packages Section
find_package(Vulkan REQUIRED)

## Compile options

set(CMAKE_CONFIGURATION_TYPES "EngineDebug;EngineRelease;GameDebug;GameRelease" CACHE STRING "" FORCE)
target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:EngineDebug>:CLZ_DEBUG CLZ_ENABLE_EDITOR CLZ_ENABLE_LOGS>
        $<$<CONFIG:EngineRelease>:CLZ_ENABLE_EDITOR CLZ_RELEASE>
        $<$<CONFIG:GameDebug>:CLZ_DEBUG CLZ_ENABLE_LOGS>
        $<$<CONFIG:GameRelease>:CLZ_RELEASE>
)

#Set compiler flags
set(CMAKE_CXX_FLAGS_ENGINEDEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_GAMEDEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_ENGINERELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_GAMERELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

# Compiler specifics
set(CLZ_SIMD_FLAGS "")
set(CLZ_DEBUG_WARNINGS "")
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	set(CLZ_DEBUG_WARNINGS -Wall -Wextra -Wsign-compare)
	set(CLZ_SIMD_FLAGS -msse4.1 -mfma)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CLZ_DEBUG_WARNINGS /W4)
    set(CLZ_SIMD_FLAGS /arch:AVX2)
endif()

target_compile_options(${PROJECT_NAME} PRIVATE
	${CLZ_SIMD_FLAGS}
	$<$<CONFIG:EngineDebug,GameDebug>:${CLZ_DEBUG_WARNINGS}>
)

## External Libs, This should be the last section, everything below this is just linking third party libraries

# Always use static libs
set(BUILD_SHARED_LIBS OFF)

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
set(LIBTYPE "STATIC" CACHE STRING "" FORCE)
add_subdirectory(external/openal-soft)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenAL)
target_compile_definitions(${PROJECT_NAME} PRIVATE AL_LIBTYPE_STATIC)

#JSON
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
add_subdirectory(external/json)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

#STB_IMAGE
target_include_directories(${PROJECT_NAME} PRIVATE external/stb)

# FASTGLTF
set(FASTGLTF_COMPILE_AS_CPP20 ON CACHE BOOL "" FORCE)
add_subdirectory(external/fastgltf)
target_link_libraries(${PROJECT_NAME} PRIVATE fastgltf)

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
#target_compile_options(imguizmo PRIVATE -Wno-error -Wno-unsafe-buffer-usage)
target_link_libraries(${PROJECT_NAME} PRIVATE imguizmo)

#BOX3D
add_subdirectory(external/box3d)
target_link_libraries(${PROJECT_NAME} PRIVATE box3d::box3d)
