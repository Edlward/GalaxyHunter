#include "testingimager.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QImage>
#include <QDebug>
#include <QThread>
#include <QPixmap>

using namespace std;

class TestingImager::Settings : public Imager::Settings {
public:
  virtual ComboSetting shutterSpeed() const { return _shutterSpeed; }
  virtual ComboSetting imageFormat() const { return _imageFormat; }
  virtual ComboSetting iso() const { return _iso; }
  virtual void setShutterSpeed(const QString &s) { _shutterSpeed.current = s; }
  virtual void setImageFormat(const QString &s) { _imageFormat.current = s; }
  virtual void setISO(const QString &s) { _iso.current = s; }
  virtual void setManualExposure(qulonglong seconds) { _manualExposure = seconds; }
  virtual qulonglong manualExposure() const { return _manualExposure; }
  virtual void setSerialShootPort(const string serialShootPort) { _serialPort = serialShootPort; }
  virtual string serialShootPort() const { return _serialPort; }
private:
  ComboSetting _shutterSpeed;
  ComboSetting _imageFormat;
  ComboSetting _iso;
  qulonglong _manualExposure;
  string _serialPort;
};


TestingImagerDriver::TestingImagerDriver(QObject *parent) :
    ImagingDriver(parent)
{
}

TestingImager::TestingImager() : Imager(), _settings(make_shared<TestingImager::Settings>())
{

}


void TestingImager::connect()
{
  emit connected();
}

void TestingImager::disconnect()
{

}

void TestingImagerDriver::scan_imagers()
{
  _imagers = {make_shared<TestingImager>()};
}


QImage TestingImager::shoot() const
{
  QString imageFile= QString(":imager/testing/%1.jpg").arg( (qrand() % 12) + 1);
  qDebug() << "loading image: " << imageFile;
  for(int i=0; i<_settings->manualExposure(); i++) {
    emit exposure_remaining(_settings->manualExposure()-i);
    thread()->msleep(1000);
  }
  QImage image(imageFile);
  return image;
}

