cmake_minimum_required(VERSION 3.25)
project(CurlozEngine VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Sources
file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE include)

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

# Always use static libs
set(BUILD_SHARED_LIBS OFF)

# External Libs, This should be the last section, everything below this is just linking third party libraries

#GLFW
add_subdirectory(external/glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

#ENTT
add_subdirectory(external/entt)
target_link_libraries(${PROJECT_NAME} PRIVATE EnTT::EnTT)

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

#GLM
add_subdirectory(external/glm)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)