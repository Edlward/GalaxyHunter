project(gulinux-astronomy-tools)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)
add_subdirectory("set-serial-signal")
add_subdirectory(scripts)
add_subdirectory(dslr-qt-shooter)


