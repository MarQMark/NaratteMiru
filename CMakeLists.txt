cmake_minimum_required(VERSION 3.20)
project(NarrateMiru)

set(CMAKE_CXX_STANDARD 20)

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

find_package(Git REQUIRED)

execute_process(
        COMMAND ${GIT_EXECUTABLE} submodule status --recursive
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE SUBMODULE_STATUS
        RESULT_VARIABLE RESULT
)

if(NOT RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to query submodule status")
endif()

# Git prefixes uninitialized submodules with '-'
string(REGEX MATCH "(^|\n)-" HAS_UNINITIALIZED "${SUBMODULE_STATUS}")

if(HAS_UNINITIALIZED)
    message(STATUS "Initializing submodules...")
    set(MAX_RETRIES 5)
    foreach(i RANGE 1 ${MAX_RETRIES})
        execute_process(
                COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                RESULT_VARIABLE RESULT
                ERROR_VARIABLE ERROR
        )

        if(RESULT EQUAL 0)
            break()
        endif()
        message(STATUS "Submodule update failed (attempt ${i}), retrying...")
        execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1)
    endforeach()
    if(NOT RESULT EQUAL 0)
        message(FATAL_ERROR "${ERROR}")
    endif()
endif()


file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(NarrateMiru ${SOURCES})

# DearImGUI
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)

add_library(imgui
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)

target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${IMGUI_DIR}/misc/cpp
)

target_include_directories(imgui PUBLIC ${IMGUI_DIR})

target_include_directories(NarrateMiru
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external
)

add_subdirectory("external/json")

if (UNIX)
    find_package(OpenGL REQUIRED)
    find_package(glfw3 REQUIRED)
    target_link_libraries(NarrateMiru
            PRIVATE GL
            PRIVATE imgui
            PRIVATE glut
            PRIVATE glfw
            PRIVATE dl
            PRIVATE X11
            PRIVATE pthread
            PRIVATE nlohmann_json::nlohmann_json
    )
endif(UNIX)

