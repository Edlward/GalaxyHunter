#include "testingimager.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QImage>
#include <QDebug>

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
  _network = new QNetworkAccessManager(this);
  connect(_network, SIGNAL(finished(QNetworkReply*)), this, SLOT(imageDownloaded(QNetworkReply*)));
}

void TestingImagerDriver::preview()
{
  QUrl imageUrl("http://lorempixel.com/400/200/");
  _network->get(QNetworkRequest{imageUrl});
}

void TestingImagerDriver::imageDownloaded(QNetworkReply* reply)
{
  if(reply->error() != QNetworkReply::NoError) {
    qDebug() << __PRETTY_FUNCTION__ << ", reply response: " << reply->errorString();
    qDebug() << reply->readAll();
    return;
  }
  qDebug() << "Download finished, loading image...";
  auto data = reply->readAll();
  for(auto header: reply->rawHeaderPairs()) {
    qDebug() << header.first << ": " << header.second;
  }
  qDebug() << data;
  QImage image;
  if(image.loadFromData(data))
    emit camera_preview(image);
}
