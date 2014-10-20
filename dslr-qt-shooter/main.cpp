#include <iostream>
#include <QApplication>
#include "dslr_shooter_window.h"

using namespace std;

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  (new DSLR_Shooter_Window())->show();
  return app.exec();
}
