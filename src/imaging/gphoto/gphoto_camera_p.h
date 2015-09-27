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
  Private(const shared_ptr<GPhotoCameraInformation> &info, ShooterSettings &shooterSettings, GPhotoCamera *q);
  string port;
  Info info;
  GPContext* context;
  Camera *camera = nullptr;
  QString outputDirectory;
  Image::ptr shootTethered(const Imager::Settings &settings);
  Image::ptr shootPreset();
  QString fixedFilename(QString fileName) const;
  QMutex &mutex;
  ShooterSettings &shooterSettings;
  Imager::Settings imagerSettings;
  
  void gphoto_error(int error_code, const QString &file, int line);
#define GPHOTO_RETURN_ERROR(error_code, ...) gphoto_error(error_code, __FILE__, __LINE__); return __VA_ARGS__ ;
#define GPHOTO_CHECK_ERROR(error_code, context, ...) if(error_code != GP_OK) { context->GPHOTO_RETURN_ERROR(error_code, __VA_ARGS__) }
#define GPHOTO_RUN(expression, context, ...) if( (error_code = expression) != GP_OK) {context->GPHOTO_RETURN_ERROR(error_code, __VA_ARGS__) }
  
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