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
  virtual void setSerialShootPort(const QString serialShootPort) { _serialPort = serialShootPort; }
  virtual QString serialShootPort() const { return _serialPort; }
private:
  ComboSetting _shutterSpeed{"1", {"1", "2", "5"}};
  ComboSetting _imageFormat{"PNG", {"PNG"}};
  ComboSetting _iso{"100", {"100", "200"}};
  qulonglong _manualExposure = 0;
  QString _serialPort = "/dev/ttyUSB0";
};


TestingImagerDriver::TestingImagerDriver(ShooterSettings &shooterSettings, QObject *parent): ImagingDriver(parent), shooterSettings{shooterSettings}
{
}

TestingImager::TestingImager(ShooterSettings &shooterSettings) : Imager(), _settings{make_shared<TestingImager::Settings>()}, shooterSettings{shooterSettings}
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
  _imagers = {make_shared<TestingImager>(shooterSettings)};
}


QImage TestingImager::shoot() const
{
  QString imageFile= QString(":imager/testing/%1.jpg").arg( (qrand() % 12) + 1);
  qDebug() << "loading image: " << imageFile;
  
  int exposure = _settings->manualExposure() == 0 ? _settings->shutterSpeed().current.toInt() : _settings->manualExposure();
  
  for(int i=0; i<exposure; i++) {
    emit exposure_remaining(exposure-i);
    QThread::currentThread()->msleep(1000);
  }
  QImage image(imageFile);
  return image;
}

