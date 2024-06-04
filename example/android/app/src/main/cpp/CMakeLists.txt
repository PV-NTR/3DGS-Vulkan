cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
        -std=c++17 -Wall -Wno-unused-variable \
        -Wno-delete-non-virtual-dtor -DVK_USE_PLATFORM_ANDROID_KHR")

project("vkrenderer")

add_definitions(-DHOST_ANDROID)
# add game-activity
find_package(game-activity REQUIRED CONFIG)
# add log lib
find_library(log-lib log)

# add asset manager
set(ASSET_MANAGER ${ANDROID_NDK}/sysroot/usr/include)

# add 3rdparty
## glm
set(3RDPARTY_DIR ../../../../../../3rdparty)
include_directories(${3RDPARTY_DIR}/glm)
## vma
set(VMA_DIR ${3RDPARTY_DIR}/vma/include)
## plog
include_directories(${3RDPARTY_DIR}/plog/include)
## imgui
set(IMGUI_DIR ${3RDPARTY_DIR}/imgui/)
file(GLOB IMGUI_SRC ${IMGUI_DIR}/*.cpp)
## vulkan headers
add_definitions(-DVULKAN_HPP_TYPESAFE_CONVERSION)
set(VULKAN_HEADER_DIR ${3RDPARTY_DIR}/vulkan-headers/include)
## glslang
set(GLSLANG_DIR ${3RDPARTY_DIR}/glslang)

# add shaderc lib
#find_package(shaderc REQUIRED CONFIG)
#set_target_properties(shaderc PROPERTIES IMPORTED_LOCATION
#        ${CMAKE_CURRENT_SOURCE_DIR}/shaderc/libs/${ANDROID_STL}/${ANDROID_ABI}/libshaderc.a)

# main library
set(RENDERER_DIR ${CMAKE_SOURCE_DIR}/../../../../../../renderer)
file(GLOB_RECURSE RENDERER_SRC ${RENDERER_DIR}/*.cpp)
add_library(${CMAKE_PROJECT_NAME} SHARED
        ${RENDERER_SRC}
        ${IMGUI_SRC}
        GameActivitySources.cpp
        AndroidMain.cpp)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ASSET_MANAGER
        ${ANDROID_NDK}/sources/android/native_app_glue
        ${VULKAN_HEADER_DIR}
        ${RENDERER_DIR}
        ${IMGUI_DIR}
        ${VMA_DIR}
        ${GLSLANG_DIR})

file(GLOB GLSLANG_LIBS ${CMAKE_SOURCE_DIR}/../../../../../../libs/glslang/android/*.a)

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
        game-activity::game-activity
        android
        log
        ${GLSLANG_LIBS})