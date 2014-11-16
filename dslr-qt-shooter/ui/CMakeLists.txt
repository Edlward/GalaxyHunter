set(dslr_ui_SOURCES dslr_shooter_window.cpp imagesettingsdialog.cpp zoomableimage.cpp ${CMAKE_SOURCE_DIR}/dslr-qt-shooter/utils/qlambdathread.cpp)
set(dslr_ui_HEADERS dslr_shooter_window.h imagesettingsdialog.h ${CMAKE_SOURCE_DIR}/dslr-qt-shooter/utils/qlambdathread.h)
set(dslr_ui_FORMS dslr_shooter_window.ui imagesettingsdialog.ui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

qt5_wrap_ui(dslr_ui_FORMS_HEADERS ${dslr_ui_FORMS})

add_library(dslr_ui STATIC ${dslr_ui_FORMS_HEADERS} ${dslr_ui_SOURCES})
set(dslr_static_LIBS ${dslr_static_LIBS} dslr_ui PARENT_SCOPE)
