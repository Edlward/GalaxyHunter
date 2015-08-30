project(GalaxyHunter)
set(PROJECT_NICE_NAME "Galaxy Hunter")
set(MAJOR_VERSION 0)
set(MINOR_VERSION 1)
set(PATCH_VERSION 0)

cmake_minimum_required(VERSION 2.8.11)
enable_testing()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")
#add_subdirectory("set-serial-signal")
#add_subdirectory(scripts)
add_subdirectory(src)


