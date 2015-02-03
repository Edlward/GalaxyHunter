find_package(PkgConfig REQUIRED)
find_library(INDI_CLIENT_LIBRARIES NAMES indiclient HINTS ${PC_INDI_LIBRARY_DIRS} )
pkg_check_modules(LIBINDI libindi REQUIRED)
include_directories(${LIBINDI_INCLUDE_DIRS})
include_directories(/usr/include/libnova) # TODO: find_package?
qt5_wrap_ui(dslr_telescope_UI 
  devicespanel.ui
  telescoperemotecontrol.ui
)
add_definitions(-DOBJECTS_DATABASE="${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/share/objects.db") # TODO: move?
add_library(dslr_telescope STATIC 
  telescopecontrol.cpp
  devicespanel.cpp
  indiclient.cpp 
  switchvectorproperty.cpp
  textvectorproperty.cpp
  numbervectorproperty.cpp
  lightvectorproperty.cpp
  devicepage.cpp
  ledindicator.cpp
  indidouble.cpp
  inumberwidget.cpp
  telescoperemotecontrol.cpp
  ${dslr_telescope_UI}
)

set(dslr_static_LIBS ${dslr_static_LIBS} dslr_telescope PARENT_SCOPE)
set(dslr_dependencies ${dslr_dependencies} ${LIBINDI_LDFLAGS} ${LIBINDI_LIBRARIES} ${INDI_CLIENT_LIBRARIES} z pthread PARENT_SCOPE)
