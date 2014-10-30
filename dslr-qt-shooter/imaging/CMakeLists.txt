find_package(PkgConfig REQUIRED)
pkg_check_modules(GPHOTO2 libgphoto2)
include_directories(${GPHOTO2_INCLUDE_DIRS})
QT4_WRAP_CPP(dslr-imaging_HEADERS_MOC gphoto.h imager_driver.h)

add_library(dslr-imaging STATIC gphoto.cpp ${dslr-imaging_HEADERS_MOC})

set(dslr-static_LIBS ${dslr-static_LIBS} dslr-imaging PARENT_SCOPE)
set(dslr-dependencies ${dslr-dependencies} ${GPHOTO2_LDFLAGS} PARENT_SCOPE)
