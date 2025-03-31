# cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)

# find_package(gazebo REQUIRED)

# FILE(GLOB SENSOR_INCLUDE "sensors/*.cc")

# include_directories(${GAZEBO_INCLUDE_DIRS})
# link_directories(${GAZEBO_LIBRARY_DIRS})
# list(APPEND CMAKE_CXX_FLAGS "${GAZEBO_CXX_FLAGS}")


# add_executable(spoofer spoofer.cc ${SENSOR_INCLUDE})
# add_executable(sensor_logger sensor_logger.cc ${SENSOR_INCLUDE})
# target_link_libraries(spoofer ${GAZEBO_LIBRARIES} pthread)
# target_link_libraries(spoofer ${GAZEBO_LIBRARIES} pthread)

cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)

# Find required packages
find_package(gazebo REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)
find_package(OpenGL REQUIRED)  # Added for ImGui rendering

# Find GLFW3 (required for ImGui)
# If your system doesn't have a FindGLFW3.cmake module, you may need to use:
# Either use pkg-config approach
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW3 REQUIRED glfw3)
# Or manually specify where GLFW is installed if the above doesn't work
# set(GLFW3_INCLUDE_DIRS "/usr/include")
# set(GLFW3_LIBRARIES "/usr/lib/x86_64-linux-gnu/libglfw.so")

# Get sensor code
FILE(GLOB SENSOR_INCLUDE "sensors/*.cc")

# ImGui source files
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Include directories
include_directories(
    ${GAZEBO_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${GLFW3_INCLUDE_DIRS}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Link directories
link_directories(
    ${GAZEBO_LIBRARY_DIRS}
    ${GLFW3_LIBRARY_DIRS}
)

# Append compiler flags
list(APPEND CMAKE_CXX_FLAGS "${GAZEBO_CXX_FLAGS}")

# Add executable
add_executable(spoofer 
    spoofer.cc 
    ${SENSOR_INCLUDE}
    ${IMGUI_SOURCES}
)

add_executable(sensor_logger sensor_logger.cc ${SENSOR_INCLUDE})
target_link_libraries(sensor_logger ${GAZEBO_LIBRARIES} pthread)

# Link libraries
target_link_libraries(spoofer 
    ${GAZEBO_LIBRARIES} 
    ${Boost_LIBRARIES}
    ${GLFW3_LIBRARIES}
    ${OPENGL_LIBRARIES}
    pthread
    dl  # Needed by some systems for dynamic linking
)
