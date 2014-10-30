set(QT_USE_QTNETWORK TRUE)
FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
ADD_DEFINITIONS(${QT_DEFINITIONS})

add_subdirectory(ui)
add_subdirectory(guider)
add_subdirectory(imaging)

ADD_EXECUTABLE(dslr-qt4-shooter main.cpp )
TARGET_LINK_LIBRARIES(dslr-qt4-shooter ${dslr-static_LIBS} ${QT_LIBRARIES} ${dslr-dependencies}  )
