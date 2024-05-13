cmake_minimum_required(VERSION 3.26 FATAL_ERROR)
#list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

#include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ProjectVersion.cmake)
#include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ProjectSettings.cmake)

cmake_minimum_required(VERSION 3.9) # 3.9+ is required for IPO/LTO checking
# rest of your CMakeLists.txt
project(
  svd2cpp
  LANGUAGES C CXX
  VERSION ${PROJECT_VERSION}
  DESCRIPTION "svd2cpp")

include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if(result)
    set_property(GLOBAL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SPDLOG_FMT_EXTERNAL ON)

add_subdirectory(libs/fmt)
include_directories(libs/fmt/include)


add_subdirectory(libs/spdlog)
add_subdirectory(libs/Catch2)
add_subdirectory(libs/tinyxml2)
add_subdirectory(libs/cxxopts)

add_subdirectory(libs/argparse)
include_directories(libs/argparse/include)

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE ${PROJECT_NAME}_HEADER CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
file(GLOB_RECURSE ${PROJECT_NAME}_HEADER2 CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)

add_subdirectory(src)
