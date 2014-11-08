#include <iostream>
#include <QApplication>
#include <Magick++.h>
#include "ui/dslr_shooter_window.h"

using namespace std;

int main(int argc, char **argv) {
  Magick::InitializeMagick(*argv);
  QApplication app(argc, argv);
  (new DSLR_Shooter_Window())->show();
  return app.exec();
}
