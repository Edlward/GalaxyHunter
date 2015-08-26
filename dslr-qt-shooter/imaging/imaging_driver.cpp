#include "imaging_driver.h"

#ifdef IMAGING_gphoto2
#include "gphoto/gphoto.h"
#warning Using gphoto2 backend
#endif
#ifdef IMAGING_testing
#include "testing/testingimager.h"
#warning Using testing backend
#endif

#include <commons/logmessage.h>
#include "commons/shootersettings.h"
#include "utils/qt.h"
#include <QDir>
#include "Qt/strings.h"

using namespace std;
using namespace std::placeholders;

ImagingDriver::ImagingDriver(QObject *parent) : QObject(parent) {
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
    connect(imager.get(), &Imager::message, bind(&ImagingDriver::camera_message, this, _1, _2));
    connect(imager.get(), &Imager::error, bind(&ImagingDriver::camera_error, this, _1, _2));
  }
  emit scan_finished();
}

QList< ImagingDriverPtr > all_imaging_drivers(ShooterSettings &shooterSettings)
{
  return {
    #ifdef IMAGING_gphoto2
    make_shared<GPhoto>(shooterSettings),
#endif
#ifdef IMAGING_testing
    make_shared<TestingImagerDriver>(shooterSettings),
#endif
  };
}


ImagingDrivers::ImagingDrivers(ShooterSettings &shooterSettings, QObject* parent): ImagingDriver(parent), imagingDrivers(all_imaging_drivers(shooterSettings))
{
  for(auto driver: imagingDrivers) {
    qDebug() << "driver: " << typeid(*driver).name();
    connect(driver.get(), &ImagingDriver::camera_connected, this, &ImagingDriver::camera_connected);
    connect(driver.get(), &ImagingDriver::imager_message, bind(&ImagingDriver::imager_message, this, _1));
  }
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

class RegisterImagePtrMetaType {
public:
  RegisterImagePtrMetaType() { qRegisterMetaType<Image::ptr>(); }
};

RegisterImagePtrMetaType __registerImagePtrMetaType;

void Image::save(const QString& directory, const QString& filename)
{
  auto path = "%1%2%3"_q % directory % QDir::separator() % (filename.isEmpty() ? originalFileName() : filename);
  save_to(path);
}

