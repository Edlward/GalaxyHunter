#ifndef GULINUX_GPHOTO_CAMERA_P_H
#define GULINUX_GPHOTO_CAMERA_P_H

#include "gphoto_camera.h"
#include "gphoto_commons.h"
#include "gphoto.h"
#include <QTemporaryFile>
#include "utils/scope.h"
#include <QThread>
#include <QImage>
#include <QDebug>

#include <Magick++.h>
#include "commons/shootersettings.h"

#include "libgphoto++/src/camera.h"
#include "libgphoto++/src/camera_filesystem.h"
#include "libgphoto++/src/camerafile.h"
#include "libgphoto++/src/exposure.h"
#include "imager.h"

using namespace std;


class CameraTempFile : public Image {
public:
  CameraTempFile(const GPhotoCPP::CameraFilePtr &camera_file);
  ~CameraTempFile();
  QTemporaryFile temp_file;
  QString mimeType() const;
  
  virtual operator QImage() const;
protected:
  virtual QString originalFileName();
  virtual void save_to(const QString &path);
private:
  const GPhotoCPP::CameraFilePtr camera_file;
};


class GPhotoCamera::Private {
public:
  Private(const GPhotoCPP::Driver::CameraFactory::ptr& info, ShooterSettings& shooterSettings, GPhotoCamera* q);
  GPhotoCPP::Driver::CameraFactory::ptr factory;
  GPhotoCPP::CameraPtr camera;
  GPhotoCPP::Camera::ControlPtr camera_control;
  GPhotoCPP::Camera::SettingsPtr camera_settings;
  QString outputDirectory;
  Image::ptr shootTethered(const Imager::Settings &settings);
  Image::ptr shootPreset();
  QString fixedFilename(QString fileName) const;
  ShooterSettings &shooterSettings;
  Imager::Settings imagerSettings;
  Info info;

  template<typename T, typename V = typename T::value_type>
  void init_combo_settings(const V &value, const T &avail, Imager::Settings::ComboSetting &combo, std::function<QString(V)> transform_f) {
    combo.available.clear();
    combo.current = transform_f(value);
    transform(begin(avail), end(avail), back_inserter(combo.available), transform_f);
  }
  
private:
  GPhotoCamera *q;
};


#endif