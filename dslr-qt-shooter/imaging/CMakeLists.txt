find_package(PkgConfig REQUIRED)
pkg_check_modules(GPHOTO2 libgphoto2)
include_directories(${GPHOTO2_INCLUDE_DIRS})
set(dslr_imaging_HEADERS imager_driver.h)
set(dslr_imaging_HEADERS imager_driver.h)

QT4_WRAP_CPP(dslr_imaging_HEADERS_MOC gphoto.h imager_driver.h)

add_library(dslr_imaging STATIC gphoto.cpp ${dslr_imaging_HEADERS_MOC})

set(dslr_static_LIBS ${dslr_static_LIBS} dslr_imaging PARENT_SCOPE)
set(dslr_dependencies ${dslr_dependencies} ${GPHOTO2_LDFLAGS} PARENT_SCOPE)
