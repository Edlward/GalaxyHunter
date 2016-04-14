#define cimg_plugin "plugins/jpeg_buffer.h"
#include <cstdio>
#include <jpeglib.h>
#include <jerror.h>
#include "CImg.h"

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
#include <imager.h>

using namespace std;



TestingImagerDriver::TestingImagerDriver(QObject* parent): ImagingDriver(parent)
{
  Q_INIT_RESOURCE(testing_imager_resources);
}

TestingImager::TestingImager() : Imager()
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
  _imagers = {make_shared<TestingImager>()};
}

class TestingImage : public Image {
public:
  TestingImage(const cimg_library::CImg< uint8_t >& image, const vector< uint8_t >& data);
};

TestingImage::TestingImage(const cimg_library::CImg< uint8_t >& image, const vector<uint8_t> &data): Image{"%1.jpg"_q % QDateTime::currentDateTime().toString(Qt::ISODate), data}
{
  this->image = image.get_normalize(0, std::numeric_limits<uint16_t>::max());
}


Imager::Info TestingImager::info() const
{
  return { "Testing Imager", "testing imager model", "simulator" };
}

Imager::Settings TestingImager::settings() const
{
  return {
    {"1", {"Bulb", "1", "2", "5"}},
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
  cimg_library::CImg<uint8_t> image_cimg;
  image_cimg.load_jpeg_buffer(reinterpret_cast<const uint8_t*>(imageData.data()), imageData.size());
  
  int pix_w = rand(0, 20);
  int pix_h = rand(0, 20);
  
  image_cimg.crop(pix_w, pix_h, image_cimg.width()-20 + pix_w, image_cimg.height()-20 + pix_h);
  image_cimg.blur(rand(0, 4), rand(0, 4));

  unsigned int jpeg_buffer_size = imageData.size()*10;
  vector<uint8_t> buffer(jpeg_buffer_size);
  image_cimg.save_jpeg_buffer(buffer.data(), jpeg_buffer_size);
  buffer.resize(jpeg_buffer_size);
  for(int i=0; i<10; i++)
    cerr << "b[" << i << "]=" << hex << static_cast<int>(buffer[i]) << " ";
  cerr << endl;
  return make_shared<TestingImage>(image_cimg, buffer);
}

