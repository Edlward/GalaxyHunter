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

using namespace std;


class CameraTempFile : public Image {
public:
  CameraTempFile(GPhotoCamera *imager);
  ~CameraTempFile();
  int save();
  CameraFile *camera_file;
  QTemporaryFile temp_file;
  QString originalName;
  operator CameraFile *() const { return camera_file; }
  operator QString() const { return path(); }
  QString mimeType() const;
  QString path() const { return temp_file.fileName(); }
  
  virtual operator QImage() const;
protected:
  virtual QString originalFileName();
  virtual void save_to(const QString &path);
private:
  GPhotoCamera *imager;
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
  GPhotoCPP::ExposurePtr exposureSetting;
  Info info;
  
  class GPhotoComboSetting {
  public:
    GPhotoComboSetting(Private *d, const QString &settingName); // Loads value from camera
    operator Imager::Settings::ComboSetting() const { return comboSetting; } // returns setting values
    void save(const Imager::Settings::ComboSetting &settings); // Saves the value to camera, returns a gphoto error code?
  private:
    void load();
    Private *d;
    const QString settingName;
    Imager::Settings::ComboSetting comboSetting;
  };
  
private:
  GPhotoCamera *q;
};

#endif