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
#include "commons/shootersettings.h"

using namespace std;

class GPhotoCamera::Settings : public Imager::Settings {
public:
  struct Set {
    int load();
    int save();
    void change(const QString &value) { setting.current = value; }
    ComboSetting setting;
    CameraWidget *widget;
    QString _original;
  };
    Settings(GPContext *context, Camera *camera, GPhotoCamera *q, QMutex &mutex);
    virtual ComboSetting imageFormat() const { return _imageFormat.setting; }
    virtual ComboSetting iso() const { return _iso.setting; }
    virtual ComboSetting shutterSpeed() const { return _shutterSpeed.setting; }
    virtual qulonglong manualExposure() const;
    virtual void setImageFormat(const QString &v) { _imageFormat.change(v); }
    virtual void setISO(const QString &v) { _iso.change(v); }
    virtual void setManualExposure(qulonglong seconds);
    virtual void setShutterSpeed(const QString &v) { _shutterSpeed.change(v); }
    virtual QString serialShootPort() const;
    virtual void setSerialShootPort(const QString serialShootPort);
    virtual ~Settings();
private:
  GPContext *context;
  Camera *camera;
  QMutex &mutex;
  CameraWidget *settings;
  GPhotoCamera *q;
  Set _imageFormat;
  Set _iso;
  Set _shutterSpeed;
};


struct CameraTempFile {
  CameraTempFile();
  ~CameraTempFile();
  int save();
  CameraFile *camera_file;
  QTemporaryFile temp_file;
  QString originalName;
  operator CameraFile *() const { return camera_file; }
  operator QString() const { return path(); }
  QString mimeType() const;
  QString path() const { return temp_file.fileName(); }
};


class GPhotoCamera::Private {
public:
  Private(const shared_ptr<GPhotoCameraInformation> &info, ShooterSettings &shooterSettings, GPhotoCamera *q)
    : model(QString::fromStdString(info->name)), port(info->port), context(info->context), mutex(info->mutex), shooterSettings{shooterSettings}, q(q) {}
  string port;
  QString model;
  QString about;
  QString summary;
  GPContext* context;
  Camera *camera = nullptr;
  qulonglong manualExposure = 0l;
  QString outputDirectory;
  QImage shootTethered();
  QImage shootPreset();
  QImage fileToImage(CameraTempFile &cameraTempFile) const;
  std::string fixedFilename(const std::string &fileName) const;
  QMutex &mutex;
  std::string serialShootPort;
  ShooterSettings &shooterSettings;
private:
  GPhotoCamera *q;
};



#endif