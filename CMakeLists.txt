cmake_minimum_required(VERSION 3.16)

project(mujoco_simulation_qt
        VERSION 0.1.0
        DESCRIPTION "A simple MuJoCo simulation application in Qt"
        HOMEPAGE_URL "https://github.com/hesic73/mujoco_simulation_qt"
        LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_path(MUJOCO_INCLUDE_DIR NAMES mujoco/mujoco.h HINTS ${CMAKE_PREFIX_PATH}/include)
if(NOT MUJOCO_INCLUDE_DIR)
    message(FATAL_ERROR "MuJoCo headers not found. Please specify the MuJoCo directory using -DCMAKE_PREFIX_PATH.")
endif()


# Find MuJoCo Library
find_library(MUJOCO_LIBRARY NAMES mujoco libmujoco HINTS ${CMAKE_PREFIX_PATH}/lib)
if(NOT MUJOCO_LIBRARY)
    message(FATAL_ERROR "MuJoCo library not found. Please specify the MuJoCo directory using -DCMAKE_PREFIX_PATH.")
endif()

# Include the MuJoCo headers
include_directories(${MUJOCO_INCLUDE_DIR})



find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
qt_standard_project_setup()

qt_add_executable(mujoco_simulation_qt
        src/main.cpp
        src/mainwindow.hpp
        src/mujocoopenglwidget.hpp
)

target_link_libraries(mujoco_simulation_qt PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        ${MUJOCO_LIBRARY}
)