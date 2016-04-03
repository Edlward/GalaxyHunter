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
  if(d->camera->settings().needs_serial_port() && !settings.serialShootPort.isEmpty())
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


CameraImage::CameraImage(const GPhotoCPP::CameraFilePtr &camera_file) : camera_file(camera_file)
{
  vector<uint8_t> data;
  camera_file->copy(data);
  auto original_file_name = originalFileName().toStdString();
  auto imageReader = GPhotoCPP::ReadImage::factory({original_file_name});
  if(!imageReader)
    return;
  auto image_decoded = imageReader->read(data, original_file_name);
  cerr << "Decoded image: " << original_file_name << ", " << image_decoded.w << "x" << image_decoded.h << "x" << image_decoded.axis << "@" << image_decoded.bpp << ", " << image_decoded.data.size() << "B" << endl;
  image = cimg_library::CImg<uint8_t>(image_decoded.w, image_decoded.h, image_decoded.bpp == 8 ? 1 : 2, image_decoded.axis == 3 ? 3 : 1);
  move(begin(image_decoded.data), end(image_decoded.data), image.begin());
//   image.save("/tmp/test.png");
  cerr << "Moved image data to cimg: " << image.width() << "x" << image.height() << "x" << image.spectrum() << "@" << image.depth() << ", size: " << image.size() << endl;
}

CameraImage::operator QImage() const {
  QImage qimage(image.width(), image.height(), QImage::Format_ARGB32);
  cimg_forXY(image, x, y) {
    auto r = image(x, y, 0);
    auto g = image.spectrum() == 3 ? image(x, y, 1) : r;
    auto b = image.spectrum() == 3 ? image(x, y, 2) : r;
    qimage.setPixel(x, y, qRgb(r, g, b));
  }
  return qimage;
}

void CameraImage::save_to(const QString& path){
  QFile file{path};
  file.write(reinterpret_cast<const char*>(original_data.data()), original_data.size());
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

