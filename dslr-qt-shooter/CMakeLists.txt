set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Sql REQUIRED)
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
include_directories(${Qt5Sql_INCLUDE_DIRS})
include_directories(${Qt5Concurrent_INCLUDE_DIRS})
include_directories(${ImageMagick_INCLUDE_DIRS})

add_subdirectory(ui)
add_subdirectory(qwt-src)
add_subdirectory(guider)
add_subdirectory(imaging)
add_subdirectory(telescope)
add_subdirectory(files)
add_subdirectory(commons)

ADD_EXECUTABLE(dslr-qt-shooter main.cpp resources.qrc )
TARGET_LINK_LIBRARIES(dslr-qt-shooter  ${dslr_static_LIBS} dsl_shooter_commons Qt5::Widgets Qt5::Network Qt5::Concurrent Qt5::Sql ${MAGICK_LDFLAGS} 
  ${dslr_dependencies} ${CFITSIO_LDFLAGS} ${Boost_LIBRARIES}  )
install(TARGETS dslr-qt-shooter RUNTIME DESTINATION bin)
add_subdirectory(tests)


