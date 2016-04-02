#ifndef GULINUX_GPHOTO_CAMERA_H
#define GULINUX_GPHOTO_CAMERA_H

#include "imaging/imaging_driver.h"
#include "dptr.h"
#include "libgphoto++/src/driver.h"

class ShooterSettings;
class GPhotoCameraInformation;
class GPhotoCamera : public Imager {
  Q_OBJECT
public:
  GPhotoCamera(const GPhotoCPP::Driver::CameraFactory::ptr &gphotoCameraInformation);
  ~GPhotoCamera();
  virtual Info info() const;
  virtual Settings settings() const;
public slots:
  virtual void connect();
  virtual void disconnect();
  virtual Image::ptr shoot(const Imager::Settings &settings) const;
private:
  D_PTR
};

#endif
