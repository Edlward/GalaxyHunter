find_package(PkgConfig REQUIRED)
pkg_check_modules(GPHOTO2 libgphoto2)
include_directories(${GPHOTO2_INCLUDE_DIRS})
set(dslr_imaging_HEADERS imager_driver.h)


set(dslr_imaging_SOURCES gphoto.cpp)
set(dslr_imaging_HEADERS ${dslr_imaging_HEADERS} gphoto.h)

QT4_WRAP_CPP(dslr_imaging_HEADERS_MOC ${dslr_imaging_HEADERS})

add_library(dslr_imaging STATIC ${dslr_imaging_SOURCES} ${dslr_imaging_HEADERS_MOC})

set(dslr_static_LIBS ${dslr_static_LIBS} dslr_imaging PARENT_SCOPE)
set(dslr_dependencies ${dslr_dependencies} ${GPHOTO2_LDFLAGS} PARENT_SCOPE)
