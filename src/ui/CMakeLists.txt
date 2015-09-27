set(dslr_ui_SOURCES
  dslr_shooter_window.cpp
  imagesettingsdialog.cpp
  messageswindow.cpp
  dslr_shooter_window.ui
  imagesettingsdialog.ui
  messageswindow.ui
  camera_setup.ui
  camerasetup.cpp
  sequenceswidget.ui
  sequenceswidget.cpp
  addsequenceitem.ui
)
set(CMAKE_AUTOUIC ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
qt5_wrap_ui(dslr_missing_ui_SOURCES sequenceswidget.ui)
add_library(dslr_ui STATIC ${dslr_ui_SOURCES} ${dslr_missing_ui_SOURCES})
set(dslr_static_LIBS ${dslr_static_LIBS} dslr_ui PARENT_SCOPE)
