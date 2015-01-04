set(dslr_imaging_HEADERS imaging_driver.h)
set(dslr_imaging_SOURCES imaging_driver.cpp file2image.cpp)

set(DEFAULT_IMAGING_BACKEND gphoto2)
set(IMAGING_BACKEND ${DEFAULT_IMAGING_BACKEND} CACHE STRING "Imaging backend to use (Default: $DEFAULT_IMAGING_BACKEND)")
add_definitions(-DIMAGING_${IMAGING_BACKEND})

if(IMAGING_BACKEND STREQUAL "gphoto2")
  add_subdirectory(gphoto)
elseif(IMAGING_BACKEND STREQUAL "testing")
  add_subdirectory(testing)
else(IMAGING_BACKEND STREQUAL "gphoto2")
    message(FATAL_ERROR "Backend ${IMAGING_BACKEND} not recognized.
Supported values:
  gphoto2
  testing")
endif(IMAGING_BACKEND STREQUAL "gphoto2")

add_library(dslr_imaging STATIC ${dslr_imaging_SOURCES} ${dslr_imaging_HEADERS_MOC})

set(dslr_static_LIBS ${dslr_static_LIBS} dslr_imaging PARENT_SCOPE)
set(dslr_dependencies ${dslr_dependencies} ${GPHOTO2_LDFLAGS} PARENT_SCOPE)
