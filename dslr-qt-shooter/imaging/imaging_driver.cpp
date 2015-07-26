#include "imaging_driver.h"

#ifdef IMAGING_gphoto2
#include "gphoto/gphoto.h"
#warning Using gphoto2 backend
#endif
#ifdef IMAGING_testing
#include "testing/testingimager.h"
#warning Using testing backend
#endif

#include <ui/logmessage.h>
using namespace std;

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
  emit imager_message(LogMessage::error(camera->model(), message) );
}

void ImagingDriver::camera_message(Imager* camera, const QString& message)
{
  emit imager_message(LogMessage::info(camera->model(), message) );
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

ImagingDrivers::ImagingDrivers(QObject* parent): ImagingDriver(parent), imagingDrivers(allDrivers())
{
  for(auto driver: imagingDrivers) {
    qDebug() << "driver: " << typeid(*driver).name();
    connect(driver.get(), SIGNAL(camera_connected()), this, SIGNAL(camera_connected()));
    connect(driver.get(), SIGNAL(imager_message(LogMessage)), this, SIGNAL(imager_message(LogMessage)));
  }
}

QList< ImagingDriverPtr > ImagingDrivers::allDrivers()
{
  return {
    #ifdef IMAGING_gphoto2
    make_shared<GPhoto>(),
#endif
#ifdef IMAGING_testing
    make_shared<TestingImagerDriver>(),
#endif
  };
}


void ImagingDrivers::scan_imagers()
{
  _imagers.clear();
  for(auto driver: imagingDrivers) {
    driver->scan();
    auto imagers = driver->imagers();
    copy(begin(imagers), end(imagers), back_inserter(_imagers));
  }
}

