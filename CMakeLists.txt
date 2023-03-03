include(FetchContent)
include(CheckCCompilerFlag)

cmake_minimum_required(VERSION 3.17)

project(vkFrame)
set(LIB_NAME vkFrame)
set(CMAKE_CXX_STANDARD 17)

# Find packages, the following line is only necessary for VCPKG users.
include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
include_directories($ENV{VCPKG_ROOT}/installed/x64-windows/include
                        $ENV{VULKAN_SDK}/Include)

add_library(
        ${LIB_NAME} STATIC
        src/vkFrame/renderer.cpp src/vkFrame/renderer.hpp
        src/vkFrame/buffer.cpp src/vkFrame/buffer.hpp
        src/vkFrame/commands.cpp src/vkFrame/commands.hpp
        src/vkFrame/swapchain.cpp src/vkFrame/swapchain.hpp
        src/vkFrame/image.cpp src/vkFrame/image.hpp
        src/vkFrame/pipeline.cpp src/vkFrame/pipeline.hpp
        src/vkFrame/renderPass.cpp src/vkFrame/renderPass.hpp
        src/vkFrame/uniformBuffer.hpp
        src/vkFrame/model.hpp
        src/vkFrame/queueFamilyIndices.hpp
        src/vkFrame/headerImpls.cpp
)

target_include_directories(${LIB_NAME} INTERFACE src/vkFrame/..)

find_package(Vulkan REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)

target_link_libraries(
        ${LIB_NAME} PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        Vulkan::Vulkan
        glm::glm
        unofficial::vulkan-memory-allocator::vulkan-memory-allocator
)

# Examples

set(ExampleNames UpdateExample CubesExample RenderTextureExample)

add_executable(UpdateExample src/examples/update.cpp)
target_link_libraries(UpdateExample ${LIB_NAME})

add_executable(CubesExample src/examples/cubes.cpp)
target_link_libraries(CubesExample ${LIB_NAME})

add_executable(RenderTextureExample src/examples/renderTexture.cpp)
target_link_libraries(RenderTextureExample ${LIB_NAME})

foreach(EXAMPLE IN LISTS ExampleNames)
        add_custom_command(
                TARGET ${EXAMPLE}
                POST_BUILD
                COMMAND
                ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
                DEPENDS ${CMAKE_CURRENT_BINARY_DIR}
        )
endforeach()