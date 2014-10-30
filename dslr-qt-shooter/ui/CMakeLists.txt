set(dslr-ui_SOURCES main.cpp dslr_shooter_window.cpp)
set(dslr-ui_HEADERS dslr_shooter_window.h )
set(dslr-ui_FORMS dslr_shooter_window.ui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

QT4_WRAP_CPP(dslr-ui_HEADERS_MOC ${dslr-ui_HEADERS})
QT4_WRAP_UI(dslr-ui_FORMS_HEADERS ${dslr-ui_FORMS})
add_library(dslr-ui STATIC dslr_shooter_window.cpp ${dslr-ui_FORMS_HEADERS} ${dslr-ui_HEADERS_MOC})
set(dslr-static_LIBS ${dslr-static_LIBS} dslr-ui PARENT_SCOPE)
