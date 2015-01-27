find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBINDI libindi REQUIRED)
include_directories(${LIBINDI_INCLUDE_DIRS})

add_library(dslr_telescope STATIC telescopecontrol.cpp)
set(dslr_static_LIBS ${dslr_static_LIBS} dslr_telescope PARENT_SCOPE)
set(dslr_dependencies ${dslr_dependencies} ${LIBINDI_LDFLAGS} PARENT_SCOPE)
