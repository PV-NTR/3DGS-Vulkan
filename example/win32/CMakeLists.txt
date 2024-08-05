set(WIN_DEMO "win32_demo")

file(GLOB_RECURSE WINDOWS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${WIN_DEMO} WIN32 ${WINDOWS_SRC})

target_include_directories(${WIN_DEMO} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
    ${RENDERER_DIR}
    ${GLSLANG_DIR}
    ${VULKAN_HEADER_DIR}
    ${VMA_DIR})

target_link_libraries(${WIN_DEMO} ${CMAKE_PROJECT_NAME})