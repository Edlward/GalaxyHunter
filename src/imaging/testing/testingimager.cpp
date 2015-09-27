#include "testingimager.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QImage>
#include <QDebug>
#include <QThread>
#include <QDir>
#include <QPixmap>
#include "utils/qt.h"
#include "Qt/strings.h"

using namespace std;



TestingImagerDriver::TestingImagerDriver(ShooterSettings &shooterSettings, QObject *parent): ImagingDriver(parent), shooterSettings{shooterSettings}
{
  Q_INIT_RESOURCE(testing_imager_resources);
}

TestingImager::TestingImager(ShooterSettings &shooterSettings) : Imager(), shooterSettings{shooterSettings}
{
  QFile file(":imager/testing/1.jpg");
  if(file.open(QIODevice::ReadOnly)) {
    imageData = file.readAll();
    qDebug() << "Reading all file data: " << imageData.size();
  }
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
  qDebug() << __PRETTY_FUNCTION__;
  _imagers = {make_shared<TestingImager>(shooterSettings)};
}

class TestingImage : public Image {
public:
  TestingImage(const QImage &image) : image(image) {}
  virtual operator QImage() const { return image; }
protected:
  virtual void save_to(const QString &path);
  virtual QString originalFileName();
private:
  QImage image;
};

QString TestingImage::originalFileName()
{
  return "%1.png"_q % QDateTime::currentDateTime().toString(Qt::ISODate);
}


void TestingImage::save_to(const QString& path)
{
  image.save(path);
}

Imager::Info TestingImager::info() const
{
  return { "Testing Imager", "testing imager model", "simulator" };
}

Imager::Settings TestingImager::settings() const
{
  return {
    {"1", {"1", "2", "5"}},
    {"PNG", {"PNG"}},
    {"100", {"100", "200", "500"}},
    false,
    1,
    "/dev/ttyUSB0",
  };
}



Image::ptr TestingImager::shoot(const Settings &settings) const
{
  QString imageFile= QString(":imager/testing/%1.jpg").arg( (qrand() % 12) + 1);
  qDebug() << "loading image: " << imageFile;
  
  int exposure = settings.manualExposure == 0 ? settings.shutterSpeed.current.toInt() : settings.manualExposureSeconds;
  
  for(int i=0; i<exposure; i++) {
    emit exposure_remaining(exposure-i);
    QThread::currentThread()->msleep(1000);
  }
  QImage image(imageFile);
  if(image.isNull())
    qDebug() << "Error loading image " << imageFile;
  return make_shared<TestingImage>(image);
}

