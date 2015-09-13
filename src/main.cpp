#include <iostream>
#include <QApplication>
#include <Magick++.h>
#include "ui/dslr_shooter_window.h"
#include "commons/version.h"

using namespace std;

int main(int argc, char **argv) {
  Magick::InitializeMagick(*argv);
  QApplication app(argc, argv);
  app.setQuitOnLastWindowClosed(true);
  app.setApplicationDisplayName(PROJECT_NICE_NAME);
  app.setApplicationName(PROJECT_NAME);
  app.setApplicationVersion(VERSION_STRING);
  DSLR_Shooter_Window::instance()->show();
  return app.exec();
}
