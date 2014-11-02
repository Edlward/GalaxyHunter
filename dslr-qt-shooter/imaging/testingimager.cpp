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

shared_ptr<ImagingDriver::Imager> TestingImagerDriver::imager() const
{
    return _imager;
}

void TestingImagerDriver::findCamera()
{
  _imager = make_shared<TestingImager>();
  emit camera_connected();
}

void TestingImagerDriver::preview()
{
  QImage image(1280, 1024, QImage::Format_RGB32);
  image.fill(QColor(qrand() % 256, qrand() % 256, qrand() % 256));
  emit camera_preview(image);
}

