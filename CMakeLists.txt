cmake_minimum_required(VERSION 3.25)
project(CurlozEngine VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


## Sources
file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE include)

## Find Packages Section
find_package(Vulkan REQUIRED)

## Compile options

# Compile Profile
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:CLZ_DEBUG>)

#Editor
option(CLZ_SHOW_EDITOR "Build with editor camera/UI support" ON)
if(CLZ_SHOW_EDITOR)
    target_compile_definitions(${PROJECT_NAME} PRIVATE CLZ_SHOW_EDITOR)
    message(STATUS "CLZ: Editor is enabled")
endif()

#SSE 4.1 instruction sets
target_compile_options(${PROJECT_NAME} PRIVATE -msse4.1)


## Always use static libs
set(BUILD_SHARED_LIBS OFF)

## External Libs, This should be the last section, everything below this is just linking third party libraries

#Vulkan
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

#GLFW
add_subdirectory(external/glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

#JOLT
set(JPH_USE_DX12 OFF)
set(JPH_USE_VK OFF)
set(JPH_USE_MTL OFF)
add_subdirectory(external/jolt/Build)
target_link_libraries(${PROJECT_NAME} PRIVATE Jolt)

#TOMLPLUSPLUS
add_subdirectory(external/tomlplusplus)
target_link_libraries(${PROJECT_NAME} PRIVATE tomlplusplus::tomlplusplus)

#LUA
add_subdirectory(external/lua)
target_link_libraries(${PROJECT_NAME} PRIVATE sol2 lua_static)
#SOL2
add_subdirectory(external/sol2)
target_link_libraries(${PROJECT_NAME} PRIVATE sol2)

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
