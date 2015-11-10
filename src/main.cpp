#include <iostream>
#include <QApplication>
#include <Magick++.h>
#include "ui/dslr_shooter_window.h"
#include "commons/version.h"
#include <iomanip>

using namespace std;

void log_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  static std::map<QtMsgType, std::string> log_levels {
    {QtFatalMsg   , "FATAL"},
    {QtCriticalMsg, "CRITICAL"},
    {QtWarningMsg , "WARNING"},
    {QtDebugMsg   , "DEBUG"},
  };
  QString position;
  if(context.file && context.line) {
    position = QString("%1:%2").arg(context.file).arg(context.line).replace(SRC_DIR, "");
  }
  QString function = context.function ? context.function : "";
//   std::cerr << qPrintable(msg) << endl;
  cerr << setw(8) << log_levels[type] << " - " /*<< qPrintable(position) << "@"*/<< qPrintable(function) << " " << qPrintable(msg) << endl;
  std::cerr.flush();
}

int main(int argc, char **argv) {
  Magick::InitializeMagick(*argv);
  QApplication app(argc, argv);
  app.setQuitOnLastWindowClosed(true);
  app.setApplicationDisplayName(PROJECT_NICE_NAME);
  app.setApplicationName(PROJECT_NAME);
  app.setApplicationVersion(VERSION_STRING);
  qInstallMessageHandler(log_handler);

  DSLR_Shooter_Window::instance()->show();
  return app.exec();
}
