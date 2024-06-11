cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 -DVK_USE_PLATFORM_WIN32_KHR")

project("vkrenderer")

add_definitions(-DHOST_WIN32 -DNOMINMAX)

set(RENDERER_DIR ../../renderer)
file(GLOB_RECURSE RENDERER_SRC ${RENDERER_DIR}/*.cpp)
file(GLOB_RECURSE WINDOWS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# add vulkan lib
find_package(Vulkan REQUIRED)

# add 3rdparty
## glm
set(3RDPARTY_DIR ${CMAKE_SOURCE_DIR}/../../3rdparty)
include_directories(${3RDPARTY_DIR}/glm)
## vma
set(VMA_DIR ${3RDPARTY_DIR}/vma/include)
# target_link_libraries(vma INTERFACE ${Vulkan_LIBRARY})
## plog
include_directories(${3RDPARTY_DIR}/plog/include)
## imgui
set(IMGUI_DIR ${3RDPARTY_DIR}/imgui/)
file(GLOB IMGUI_SRC ${IMGUI_DIR}/*.cpp)
## vulkan headers
set(VULKAN_HEADER_DIR ${3RDPARTY_DIR}/vulkan-headers/include)
## glslang
set(GLSLANG_DIR ${3RDPARTY_DIR}/glslang)

add_executable(${CMAKE_PROJECT_NAME} WIN32
        ${IMGUI_SRC}
        ${RENDERER_SRC}
        ${WINDOWS_SRC})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${IMGUI_DIR}
        ${RENDERER_DIR}
        ${GLSLANG_DIR}
        ${VULKAN_HEADER_DIR}
        ${VMA_DIR})

link_directories(${CMAKE_SOURCE_DIR}/../../libs/glslang/win32/)

file(GLOB GLSLANG_LIBS ${CMAKE_SOURCE_DIR}/../../libs/glslang/win32/*.lib)

target_link_libraries(${CMAKE_PROJECT_NAME} ${GLSLANG_LIBS})