#include "gphoto_camera_p.h"
#include "serialshoot.h"
#include "libgphoto++/src/utils/read_image.h"
#include <file2image.h>
#include "imager.h"
#include <iostream>
#include <QTimer>
#include <QDir>

#include "utils/qt.h"
#include <sstream>
#include "Qt/strings.h"
#include <QFuture>
#include <QMimeDatabase>
using namespace std;
using namespace std::placeholders;

GPhotoCamera::Private::Private ( const GPhotoCPP::Driver::CameraFactory::ptr& info, GPhotoCamera* q )
  : factory{info}, q(q)
{
  this->info.model = QString::fromStdString(info->name());
}


GPhotoCamera::GPhotoCamera(const GPhotoCPP::Driver::CameraFactory::ptr& gphotoCameraInformation)
  : dptr(gphotoCameraInformation, this)
{
}



void GPhotoCamera::connect()
{
  d->camera = *d->factory;
  if(d->camera) {
    // TODO: add logger
    d->info.model = QString::fromStdString(d->factory->name());
    d->info.summary = QString::fromStdString(d->camera->summary());
    
    function<QString(string)> transform_f = bind(&QString::fromStdString, _1);
    d->init_combo_settings(d->camera->settings().iso(), d->camera->settings().iso_choices(), d->imagerSettings.iso, transform_f);
    d->init_combo_settings(d->camera->settings().format(), d->camera->settings().format_choices(), d->imagerSettings.imageFormat, transform_f);
    function<QString(GPhotoCPP::Exposure::Value)> transform_value = [](const GPhotoCPP::Exposure::Value &v){ return QString::fromStdString(v.text); };
    d->init_combo_settings(d->camera->settings().exposure()->value(), d->camera->settings().exposure()->values(), d->imagerSettings.shutterSpeed, transform_value);
    emit connected();
  }
}

Imager::Settings GPhotoCamera::settings() const
{
    return d->imagerSettings;
}



void GPhotoCamera::disconnect()
{
  d->camera.reset();
  emit disconnected();
}

Image::ptr GPhotoCamera::shoot(const Imager::Settings &settings) const
{
  qDebug() << "setting camera: " << settings;
  d->camera->settings().set_iso(settings.iso.current.toStdString());
  d->camera->settings().set_format(settings.imageFormat.current.toStdString());
  GPhotoCPP::milliseconds exposure;
  if(d->camera->settings().needs_serial_port() && !settings.serialShootPort.isEmpty() && QFile::exists(settings.serialShootPort))
    d->camera->settings().set_serial_port(settings.serialShootPort.toStdString());

  if(settings.manualExposure) {
    exposure = GPhotoCPP::seconds{settings.manualExposureSeconds};
  } else {
    auto exposures = d->camera->settings().exposure()->values();
    auto exposure_v = find_if(begin(exposures), end(exposures), [&](const GPhotoCPP::Exposure::Value &v){ return v.text == settings.shutterSpeed.current.toStdString(); });
    exposure = (*exposure_v).duration();
  }
  auto shot = d->camera->control().shoot(exposure, settings.mirrorLock);
  shot->camera_file().wait();
  return make_shared<CameraImage>(shot->camera_file().get());
}

#include "c++/stlutils.h"

CameraImage::CameraImage(const GPhotoCPP::CameraFilePtr &camera_file) : camera_file(camera_file)
{
  camera_file->copy(original_data);
  auto original_file_name = originalFileName().toStdString();
  auto imageReader = GPhotoCPP::ReadImage::factory({original_file_name});
  if(!imageReader)
    return;
  benchmark_start(decode_image, 1);
  auto image_decoded = imageReader->read(original_data, original_file_name);
  benchmark_end(decode_image);
  cerr << "Decoded image: " << original_file_name << ", " << image_decoded.w << "x" << image_decoded.h << "x" << image_decoded.channels.size() << "@" << image_decoded.bpp << endl;
  
  benchmark_start(merge_channels, 1);
  cimg_library::CImgList<uint16_t> images;
  for(auto channel: image_decoded.channels) {
    auto channel_image = cimg_library::CImg<uint16_t>(image_decoded.w, image_decoded.h);
    copy_data(channel.second, channel_image);
//     move(begin(channel.second), end(channel.second), channel_image.begin());
    images.push_back(channel_image);
  }
  image = images.get_append('c');
  benchmark_end(merge_channels);
//   benchmark_scope(save_test_img, 1);
//   image.save("/tmp/test.png");
  cerr << "Saved image data to cimg: " << image.width() << "x" << image.height() << "x" << image.spectrum() << "@" << image.depth() << ", size: " << image.size() << endl;
}


void CameraImage::copy_data(const vector< uint8_t >& data, CameraImage::CImgImage& dest) const
{
  if(data.size() == dest.width() * dest.height())
    transform(data.begin(), data.end(), dest.begin(), [](uint8_t byte) { return byte * 256; });
  else {
    const uint16_t *data_ptr = reinterpret_cast<const uint16_t*>(data.data());
    copy(data_ptr, data_ptr + data.size()/2, dest.begin());
  }
}


CameraImage::operator QImage() const {
  QImage qimage(image.width(), image.height(), QImage::Format_RGB32);

  auto channels = (image.get_normalize(0, 255)).get_split('c');
  cerr << "image channels: " << channels.size() << endl;
  cimg_forXY(channels[0], x, y) {
    auto r = channels[0](x, y);
    auto g = channels.size() == 3 ? channels[1](x, y) : r;
    auto b = channels.size() == 3 ? channels[2](x, y) : r;
    qimage.setPixel(x, y, qRgb(r, g, b));
  }
  return qimage;
}

void CameraImage::save_to(const QString& path){
  QFile file{path};
  if(file.open(QIODevice::WriteOnly)) {
    auto written = file.write(reinterpret_cast<const char*>(original_data.data()), original_data.size());
    qDebug() << "written" << written << "bytes out of" << original_data.size() << "bytes to" << path;
  }
}

QString CameraImage::originalFileName() const
{
  return QString::fromStdString( camera_file->file() );
}


// TODO: add picture deletion from camera


GPhotoCamera::~GPhotoCamera()
{
}





CameraImage::~CameraImage()
{
}

Imager::Info GPhotoCamera::info() const
{
  return d->info;
}

