#include "gphoto_camera_p.h"
#include "serialshoot.h"
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

CameraImage::operator QImage() const {
  try {
    QImage image;
    QFileInfo fileInfo(originalFileName());
    File2Image file2image(image);
    file2image.load(temp_file.fileName(), fileInfo.suffix().toLower());
    return image;
  } catch(std::exception &e) {
    // TODO: error report
//       imager->error(imager, QString("Error converting image: %1").arg(e.what()));
      return QImage();
  }
}

void CameraImage::save_to(const QString& path){
  QFile file( temp_file.fileName() );
  file.copy(path);
  // TODO: error log
//   if(file.copy(path))
//     imager->message(imager, "Saved image to %1"_q % path);
//   else
//     imager->error(imager, "Error saving temporary image %1 to %2"_q % temp_file.fileName() % path);
}

QString CameraImage::originalFileName() const
{
  return QString::fromStdString( camera_file->file() );
}


// TODO: add picture deletion from camera


GPhotoCamera::~GPhotoCamera()
{
}




CameraImage::CameraImage(const GPhotoCPP::CameraFilePtr &camera_file) : camera_file(camera_file)
{
  vector<uint8_t> data;
  camera_file->copy(data);
  temp_file.open();
  temp_file.close();
  temp_file.setAutoRemove(true);
  camera_file->save(temp_file.fileName().toStdString());
}


CameraImage::~CameraImage()
{
  qDebug() << __PRETTY_FUNCTION__ ;
}

QString CameraImage::mimeType() const
{
  static QMimeDatabase db;
  auto mime = db.mimeTypeForFile(temp_file.fileName(), QMimeDatabase::MatchContent);
  return mime.name();
}

Imager::Info GPhotoCamera::info() const
{
  return d->info;
}

