find_package(PkgConfig REQUIRED)
pkg_check_modules(GPHOTO2 libgphoto2 REQUIRED)
message("GPhoto2 _LDFLAGS: ${GPHOTO2_LDFLAGS}")
message("GPhoto2 _LDFLAGS_OTHER: ${GPHOTO2_LDFLAGS_OTHER}")
message("GPhoto2 _CFLAGS: ${GPHOTO2_CFLAGS}")
message("GPhoto2 _CFLAGS_OTHER: ${GPHOTO2_CFLAGS_OTHER}")

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
