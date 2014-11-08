#include "imaging_driver.h"

#ifdef IMAGING_gphoto2
#include "gphoto.h"
#warning Using gphoto2 backend
#endif
#ifdef IMAGING_testing
#include "testingimager.h"
#warning Using testing backend
#endif


ImagingDriver::ImagingDriver(QObject *parent) : QObject(parent) {
}

ImagingDriver *ImagingDriver::imagingDriver(QObject *parent) {
#ifdef IMAGING_gphoto2
    return new GPhoto(parent);
#endif
#ifdef IMAGING_testing
    return new TestingImagerDriver(parent);
#endif
}

void ImagingDriver::camera_error(Imager* camera, const QString& message)
{
  emit imager_error(QString("%1: %2").arg(camera->model()).arg(message));
}

void ImagingDriver::camera_message(Imager* camera, const QString& message)
{
  emit imager_message(QString("%1: %2").arg(camera->model()).arg(message));
}

void ImagingDriver::scan()
{
  this->_imagers.clear();
  scan_imagers();
  for(auto imager: _imagers) {
    connect(imager.get(), SIGNAL(message(Imager*, QString)), this, SLOT(camera_message(Imager*,QString)));
    connect(imager.get(), SIGNAL(error(Imager*, QString)), this, SLOT(camera_error(Imager*,QString)));
  }
  emit scan_finished();
}