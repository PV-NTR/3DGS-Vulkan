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

# main library
set(RENDERER_DIR ../../../../../../renderer)
file(GLOB_RECURSE RENDERER_SRC ${RENDERER_DIR}/*.cpp)
add_library(${CMAKE_PROJECT_NAME} SHARED
        ${RENDERER_SRC}
        GameActivitySources.cpp
        AndroidMain.cpp)


# add vulkan hpp
add_definitions(-DVULKAN_HPP_TYPESAFE_CONVERSION)
set(VULKAN_DIR ${ANDROID_NDK}/sources/third_party/vulkan/src)
set(Vulkan_INCLUDE_DIR ${VULKAN_DIR}/include)

# add asset manager
set(ASSET_MANAGER ${ANDROID_NDK}/sysroot/usr/include)

# add 3rdparty
## glm
set(3RDPARTY_DIR ../../../../../../3rdparty)
include_directories(${3RDPARTY_DIR}/glm)
## vma
add_library(vma INTERFACE)
set(VMA_DIR ${3RDPARTY_DIR}/vma/include)
target_sources(vma INTERFACE ${VMA_DIR}/vk_mem_alloc.h)
target_include_directories(vma SYSTEM INTERFACE ${VMA_DIR})
# target_link_libraries(vma INTERFACE vulkan)
## plog
include_directories(${3RDPARTY_DIR}/plog/include)

# add shaderc lib
#find_package(shaderc REQUIRED CONFIG)
#set_target_properties(shaderc PROPERTIES IMPORTED_LOCATION
#        ${CMAKE_CURRENT_SOURCE_DIR}/shaderc/libs/${ANDROID_STL}/${ANDROID_ABI}/libshaderc.a)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${ANDROID_NDK}/sources/android/native_app_glue
        ${VULKAN_DIR}/include
        ASSET_MANAGER
        ${RENDERER_DIR})

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
        game-activity::game-activity
        vma
        android
        log)