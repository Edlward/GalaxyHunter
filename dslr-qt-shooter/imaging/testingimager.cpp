#include "testingimager.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QImage>
#include <QDebug>
#include <QPixmap>

using namespace std;

TestingImagerDriver::TestingImagerDriver(QObject *parent) :
    ImagingDriver(parent)
{
}


void TestingImager::connect()
{
  emit connected();
}

void TestingImager::disconnect()
{

}

void TestingImagerDriver::scan()
{
  _imagers = {make_shared<TestingImager>()};
  emit scan_finished();
}


void TestingImager::shootPreview()
{
  QString imageFile= QString(":imager/testing/%1.jpg").arg( (qrand() % 4) + 1);
  qDebug() << "loading image: " << imageFile;
  QImage image(imageFile);
  emit preview(image);
}
