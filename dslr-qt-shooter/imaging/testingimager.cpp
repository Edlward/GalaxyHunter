#include "testingimager.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QImage>
#include <QDebug>
#include <QPixmap>

using namespace std;

TestingImagerDriver::TestingImagerDriver(QObject *parent) :
    ImagingDriver(parent), _imager(new TestingImager)
{
}

void TestingImagerDriver::scan()
{
  qDebug() << __PRETTY_FUNCTION__;
  _imager = make_shared<TestingImager>();
  _imagers = {_imager};
  emit scan_finished();
}

void TestingImagerDriver::preview()
{
  QString imageFile= QString(":imager/testing/%1.jpg").arg( (qrand() % 4) + 1);
  qDebug() << "loading image: " << imageFile;
  QImage image(imageFile);
  emit camera_preview(image);
}

