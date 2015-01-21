set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(Magick REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread unit_test_framework system filesystem)
# TODO: check for ufraw-batch app, either in cmake or runtime

find_package(PkgConfig REQUIRED)
pkg_check_modules(CFITSIO cfitsio REQUIRED)
include_directories(${CFITSIO_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Network_INCLUDE_DIRS})
include_directories(${Qt5Concurrent_INCLUDE_DIRS})
include_directories(${ImageMagick_INCLUDE_DIRS})
add_subdirectory(ui)
add_subdirectory(qwt-src)
add_subdirectory(guider)
add_subdirectory(imaging)
qt5_add_resources(dslr_qt_shooter_RESOURCES_RCC resources.qrc )

ADD_EXECUTABLE(dslr-qt-shooter main.cpp ${dslr_qt_shooter_RESOURCES_RCC} )
TARGET_LINK_LIBRARIES(dslr-qt-shooter ${dslr_static_LIBS} Qt5::Widgets Qt5::Network Qt5::Concurrent ${MAGICK_LDFLAGS} 
  ${dslr_dependencies} ${CFITSIO_LDFLAGS} ${Boost_LIBRARIES}  )

add_subdirectory(tests)
