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
  QString imageFile= QString(":imager/testing/%1.jpg").arg( (qrand() % 4) + 1);
  qDebug() << "loading image: " << imageFile;
  QImage image(imageFile);
  emit camera_preview(image);
}

