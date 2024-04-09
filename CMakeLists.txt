cmake_minimum_required(VERSION 3.16)

project(QMuJoCoSim
        VERSION 0.1.0
        DESCRIPTION "A simple MuJoCo simulator implemented in Qt"
        HOMEPAGE_URL "https://github.com/hesic73/QMuJoCoSim"
        LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_path(MUJOCO_INCLUDE_DIR NAMES mujoco/mujoco.h HINTS ${CMAKE_PREFIX_PATH}/include)
if (NOT MUJOCO_INCLUDE_DIR)
    message(FATAL_ERROR "MuJoCo headers not found. Please specify the MuJoCo directory using -DCMAKE_PREFIX_PATH.")
endif ()


# Find MuJoCo Library
find_library(MUJOCO_LIBRARY NAMES mujoco libmujoco HINTS ${CMAKE_PREFIX_PATH}/lib)
if (NOT MUJOCO_LIBRARY)
    message(FATAL_ERROR "MuJoCo library not found. Please specify the MuJoCo directory using -DCMAKE_PREFIX_PATH.")
endif ()

# Include the MuJoCo headers
include_directories(${MUJOCO_INCLUDE_DIR}
        src)


find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
qt_standard_project_setup()

qt_add_executable(QMuJoCoSim
        src/main.cpp
        src/mainwindow.hpp
        src/mujoco_opengl_window.hpp
        src/my_window_container.hpp
        src/panel_sections/collapsible_section.h
        src/panel_sections/collapsible_section.cpp
        src/control_panel.hpp
        src/panel_sections/rendering_section.hpp
        src/my_widgets/toggling_button.hpp
        src/utils.hpp
        src/history_buffer.hpp
        src/panel_sections/simulation_section.hpp
        src/my_widgets/label_slider.hpp
)

qt_add_resources(QMuJoCoSim "resources" # This is an arbitrary identifier
        PREFIX "/"              # This is the prefix you'll use to access the resources
        FILES assets/icon.png)        # List of files to add; adjust paths as necessary

target_link_libraries(QMuJoCoSim PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        ${MUJOCO_LIBRARY}
)

enable_testing()

# Assuming Catch2 is a submodule or already present in your project
add_subdirectory(Catch2)

# Add test subdirectory
add_subdirectory(test)