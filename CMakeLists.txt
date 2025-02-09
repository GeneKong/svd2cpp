cmake_minimum_required(VERSION 3.26 FATAL_ERROR)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ProjectVersion.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ProjectSettings.cmake)

project(
  svd2cpp
  LANGUAGES C CXX
  VERSION ${PROJECT_VERSION}
  DESCRIPTION "svd2cpp")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(libs/spdlog)
add_subdirectory(libs/Catch2)
add_subdirectory(libs/tinyxml2)
add_subdirectory(libs/cxxopts)

add_subdirectory(libs/fmt)
include_directories(libs/fmt/include)

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE ${PROJECT_NAME}_HEADER CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
file(GLOB_RECURSE ${PROJECT_NAME}_HEADER2 CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)

add_subdirectory(src)
