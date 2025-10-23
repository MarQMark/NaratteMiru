cmake_minimum_required(VERSION 3.29)
project(NaratteMiru)

set(CMAKE_CXX_STANDARD 20)

#add_executable(NaratteMiru src/main.cpp)

# GLFW
add_subdirectory(extern/glfw)

# Dear ImGui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui)
set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw)

# Main executable
add_executable(NaratteMiru src/main.cpp)
target_link_libraries(NaratteMiru PUBLIC imgui glfw)

# Linux: link OpenGL
if(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)

    target_link_libraries(NaratteMiru PRIVATE
            imgui
            glfw
            OpenGL::GL
            X11
            Xrandr
            Xi
            Xxf86vm
            Xcursor
    )
endif()

