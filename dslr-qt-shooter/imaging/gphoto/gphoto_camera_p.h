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
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>


using namespace std;

class GPhotoCamera::Settings : public Imager::Settings {
public:
    Settings(GPContext *context, Camera *camera, GPhotoCamera *q);
    virtual ComboSetting imageFormat() const { return _imageFormat; }
    virtual ComboSetting iso() const { return _iso; }
    virtual ComboSetting shutterSpeed() const { return _shutterSpeed; }
    virtual uint64_t manualExposure() const;
    virtual void setImageFormat(const QString&);
    virtual void setISO(const QString&);
    virtual void setManualExposure(uint64_t seconds);
    virtual void setShutterSpeed(const QString&);
    virtual void apply();
    virtual void reload();
    virtual ~Settings();
private:
  GPContext *context;
  Camera *camera;
  GPhotoCamera *q;
  ComboSetting _imageFormat;
  ComboSetting _iso;
  ComboSetting _shutterSpeed;
  bool changed = false;
  CameraWidget *settings = nullptr;
  CameraWidget *isoWidget = nullptr;
  CameraWidget *imageFormatWidget = nullptr;
  CameraWidget *shutterSpeedWidget = nullptr;
  int loadComboSetting(ComboSetting &setting, CameraWidget *widget);
};


class GPhotoCamera::Private {
public:
  Private(const shared_ptr<GPhotoCameraInformation> &info, GPhotoCamera *q)
    : model(QString::fromStdString(info->name)), port(info->port), context(info->context), q(q) {}
  string port;
  QString model;
  QString about;
  QString summary;
  GPContext* context;
  Camera *camera = nullptr;
  uint64_t manualExposure = 0l;
  QString outputDirectory;
  void shootTethered();
  void shootPreset();
  void deletePicturesOnCamera(const CameraFilePath &camera_remote_file);
  std::string fixedFilename(const std::string &fileName) const;
private:
  GPhotoCamera *q;
};


struct CameraTempFile {
  CameraTempFile();
  ~CameraTempFile();
  int save();
  CameraFile *camera_file;
  QTemporaryFile temp_file;
  operator CameraFile *() const { return camera_file; }
  operator QString() const { return path(); }
  QString mimeType() const;
  QString path() const { return temp_file.fileName(); }
};


#endif