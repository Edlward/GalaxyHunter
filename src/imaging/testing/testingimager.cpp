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
#include "opencv2/opencv.hpp"

using namespace std;



TestingImagerDriver::TestingImagerDriver(ShooterSettings &shooterSettings, QObject *parent): ImagingDriver(parent), shooterSettings{shooterSettings}
{
  Q_INIT_RESOURCE(testing_imager_resources);
}

TestingImager::TestingImager(ShooterSettings &shooterSettings) : Imager(), shooterSettings{shooterSettings}
{
  QFile file(":imager/testing/image.jpg");
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


int TestingImager::rand(int a, int b) const
{
   return qrand() % ((b + 1) - a) + a;
}


Image::ptr TestingImager::shoot(const Settings &settings) const
{ 
  int exposure = settings.manualExposure == 0 ? settings.shutterSpeed.current.toInt() : settings.manualExposureSeconds;
  
  for(int i=0; i<exposure; i++) {
    emit exposure_remaining(exposure-i);
    QThread::currentThread()->msleep(1000);
  }
  cv::Mat cv_image = cv::imdecode(cv::InputArray{imageData.data(), imageData.size()}, CV_LOAD_IMAGE_COLOR);
  cv::Mat cropped, blurred, result;
  int h = cv_image.rows;
  int w = cv_image.cols;
  
  int pix_w = rand(0, 20);
  int pix_h = rand(0, 20);

  cv::Rect crop_rect(0, 0, w, h);
  crop_rect -= cv::Size{20, 20};
  crop_rect += cv::Point{pix_w, pix_h};
  cropped = cv_image(crop_rect);
  if(rand(0, 5) > 2) {
      auto ker_size = rand(1, 17);
      cv::blur(cropped, blurred, {ker_size, ker_size});
  } else {
      cropped.copyTo(blurred);
  }
  
  auto image_copy = new cv::Mat;
  blurred.copyTo(*image_copy);
  QImage image{image_copy->data, image_copy->cols, image_copy->rows, image_copy->step, QImage::Format_RGB888, [](void *data){ delete reinterpret_cast<cv::Mat*>(data); }, image_copy};
  return make_shared<TestingImage>(image);
}

