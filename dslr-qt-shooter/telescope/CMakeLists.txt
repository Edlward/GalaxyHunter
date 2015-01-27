find_package(PkgConfig REQUIRED)
find_library(INDI_CLIENT_LIBRARIES NAMES indiclient HINTS ${PC_INDI_LIBRARY_DIRS} )
pkg_check_modules(LIBINDI libindi REQUIRED)
include_directories(${LIBINDI_INCLUDE_DIRS})

add_library(dslr_telescope STATIC telescopecontrol.cpp)
set(dslr_static_LIBS ${dslr_static_LIBS} dslr_telescope PARENT_SCOPE)
set(dslr_dependencies ${dslr_dependencies} ${LIBINDI_LDFLAGS} ${INDI_CLIENT_LIBRARIES} z PARENT_SCOPE)
