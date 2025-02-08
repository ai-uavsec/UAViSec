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

find_package(gazebo REQUIRED)

FILE(GLOB SENSOR_INCLUDE "sensors/*.cc")

include_directories(${GAZEBO_INCLUDE_DIRS})
link_directories(${GAZEBO_LIBRARY_DIRS})
list(APPEND CMAKE_CXX_FLAGS "${GAZEBO_CXX_FLAGS}")

add_executable(spoofer spoofer.cc ${SENSOR_INCLUDE})
add_executable(sensor_logger sensor_logger.cc ${SENSOR_INCLUDE})

target_link_libraries(sensor_logger ${GAZEBO_LIBRARIES} pthread)
target_link_libraries(spoofer ${GAZEBO_LIBRARIES} pthread)