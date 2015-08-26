set(dslr_ui_SOURCES
  dslr_shooter_window.cpp
  imagesettingsdialog.cpp
  messageswindow.cpp
  dslr_shooter_window.ui
  imagesettingsdialog.ui
  messageswindow.ui
  camera_setup.ui
  camerasetup.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_library(dslr_ui STATIC ${dslr_ui_FORMS_HEADERS} ${dslr_ui_SOURCES})
set(dslr_static_LIBS ${dslr_static_LIBS} dslr_ui PARENT_SCOPE)
