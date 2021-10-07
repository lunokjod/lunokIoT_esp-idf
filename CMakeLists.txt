# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
set(CMAKE_VERBOSE_MAKEFILE ON)
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(lunokiot-idf)
#EXCLUDE_COMPONENTS