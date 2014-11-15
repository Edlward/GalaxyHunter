#ifndef GULINUX_GPHOTO_CAMERA_H
#define GULINUX_GPHOTO_CAMERA_H

#include "imaging/imaging_driver.h"
#include <memory>

class GPhotoCameraInformation;
class GPhotoCamera : public Imager {
  Q_OBJECT
public:
  GPhotoCamera(const std::shared_ptr<GPhotoCameraInformation> &gphotoCameraInformation);
  ~GPhotoCamera();
  virtual QString summary() const;
  virtual QString model() const;
  virtual QString about() const;
  virtual ComboSetting shutterSpeed() const;
  virtual ComboSetting imageFormat() const;
  virtual ComboSetting iso() const;
public slots:
  virtual void connect();
  virtual void disconnect();
  virtual void shoot();
private:
  class Private;
  friend class Private;
  std::unique_ptr<Private> const d;
};

#endif
