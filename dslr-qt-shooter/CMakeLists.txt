set(QT_USE_QTNETWORK TRUE)
FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
ADD_DEFINITIONS(${QT_DEFINITIONS})

find_package(PkgConfig REQUIRED)
include_directories(${GPHOTO2_INCLUDE_DIRS})
pkg_check_modules(GPHOTO2 libgphoto2)

SET(dslr-qt4-shooter_SOURCES main.cpp dslr_shooter_window.cpp linguider.cpp gphoto.cpp)
SET(dslr-qt4-shooter_HEADERS dslr_shooter_window.h linguider.h gphoto.h)
set(dslr-qt4-shooter_FORMS dslr_shooter_window.ui)


QT4_WRAP_CPP(dslr-qt4-shooter_HEADERS_MOC ${dslr-qt4-shooter_HEADERS})
QT4_WRAP_UI(dslr-qt4-shooter_FORMS_HEADERS ${dslr-qt4-shooter_FORMS})
ADD_EXECUTABLE(dslr-qt4-shooter ${dslr-qt4-shooter_SOURCES} ${dslr-qt4-shooter_HEADERS_MOC} ${dslr-qt4-shooter_FORMS_HEADERS})
TARGET_LINK_LIBRARIES(dslr-qt4-shooter ${QT_LIBRARIES} ${GPHOTO2_LDFLAGS})
