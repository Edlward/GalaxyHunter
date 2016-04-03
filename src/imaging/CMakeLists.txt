set(dslr_imaging_HEADERS imaging_driver.h)
set(dslr_imaging_SOURCES 
  imaging_driver.cpp
  imager.cpp
  file2image.cpp 
  focus.cpp
  focus/fitshistogram.cpp
  focus/fitsimage.cpp
  imagingmanager.cpp
  imagingsequence.cpp
)

option(testing_imager_enabled "Enable testing imager backend" OFF)
if(testing_imager_enabled)
  add_subdirectory(testing)
  add_definitions(-DIMAGING_testing)
endif(testing_imager_enabled)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPHOTO2 libgphoto2)
if(GPHOTO2_FOUND)
  find_package(JPEG REQUIRED)
  set(RAW_LIBRARIES raw) # TODO: find_package
  add_subdirectory(gphoto)
  add_definitions(-DIMAGING_gphoto2)
  set(gphoto++_library gphoto++)
endif(GPHOTO2_FOUND)


add_library(dslr_imaging STATIC ${dslr_imaging_SOURCES} ${dslr_imaging_HEADERS_MOC})

set(dslr_static_LIBS ${dslr_static_LIBS} dslr_imaging ${gphoto++_library} PARENT_SCOPE)
set(dslr_dependencies ${dslr_dependencies} ${GPHOTO2_LDFLAGS} ${JPEG_LIBRARIES} ${RAW_LIBRARIES} PARENT_SCOPE)
