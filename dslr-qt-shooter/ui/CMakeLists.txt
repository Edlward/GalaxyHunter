set(dslr_ui_SOURCES main.cpp dslr_shooter_window.cpp)
set(dslr_ui_HEADERS dslr_shooter_window.h )
set(dslr_ui_FORMS dslr_shooter_window.ui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

qt5_wrap_ui(dslr_ui_FORMS_HEADERS ${dslr_ui_FORMS})

add_library(dslr_ui STATIC ${dslr_ui_FORMS_HEADERS} ${dslr_ui_HEADERS_MOC} dslr_shooter_window.cpp)
set(dslr_static_LIBS ${dslr_static_LIBS} dslr_ui PARENT_SCOPE)
