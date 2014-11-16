find_package(PkgConfig REQUIRED)
pkg_check_modules(GPHOTO2 libgphoto2)
include_directories(${GPHOTO2_INCLUDE_DIRS})
set(dslr_imaging_SOURCES ${dslr_imaging_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/gphoto.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/gphoto_camera.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/serialshoot.cpp  
  PARENT_SCOPE)
set(dslr_imaging_HEADERS ${dslr_imaging_HEADERS}
  ${CMAKE_CURRENT_SOURCE_DIR}/gphoto.h
  ${CMAKE_CURRENT_SOURCE_DIR}/gphoto_camera.h
  PARENT_SCOPE)