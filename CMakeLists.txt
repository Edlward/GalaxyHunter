project(gulinux-astronomy-tools)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

cmake_minimum_required(VERSION 2.8.11)
enable_testing()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")
add_subdirectory("set-serial-signal")
add_subdirectory(scripts)
add_subdirectory(dslr-qt-shooter)


