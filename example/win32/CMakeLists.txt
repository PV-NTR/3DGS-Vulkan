cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
        -std=c++17 -Wall -Wno-unused-variable \
        -Wno-delete-non-virtual-dtor -DVK_USE_PLATFORM_ANDROID_KHR")

project("vkrenderer")

# add log lib
find_library(log-lib log)

# main library
set(RENDERER_DIR ../../renderer)
file(GLOB_RECURSE RENDERER_SRC ${RENDERER_DIR}/*.cpp)
add_executable(${CMAKE_PROJECT_NAME}
        ${RENDERER_SRC}
        WinMain.cpp)

# add vulkan hpp
find_package(Vulkan REQUIRED)

# add asset manager
set(ASSET_MANAGER ${ANDROID_NDK}/sysroot/usr/include)

# add shaderc lib
#find_package(shaderc REQUIRED CONFIG)
#set_target_properties(shaderc PROPERTIES IMPORTED_LOCATION
#        ${CMAKE_CURRENT_SOURCE_DIR}/shaderc/libs/${ANDROID_STL}/${ANDROID_ABI}/libshaderc.a)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ASSET_MANAGER)

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
        ${Vulkan_LIBRARY})