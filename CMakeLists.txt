cmake_minimum_required(VERSION 3.20)
project(mosek_example)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(MOSEK REQUIRED)

include(CTest)
add_subdirectory(tests)
