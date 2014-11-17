#ifndef GULINUX_GPHOTO_CAMERA_P_H
#define GULINUX_GPHOTO_CAMERA_P_H

#include "gphoto_camera.h"
#include "gphoto_commons.h"
#include <QTemporaryFile>
#include "utils/scope.h"
#include <QThread>
#include <QImage>
#include <QDebug>

#include <Magick++.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>


using namespace std;


struct CameraSetting : enable_shared_from_this<CameraSetting> {
  int id;
  string name;
  string label;
  string info;
  CameraWidgetType type;
  string path() const;
  vector<shared_ptr<CameraSetting>> children;
  shared_ptr<CameraSetting> parent;
  vector<string> choices;
  string value;
  int choice;
  struct Range {
    float min, max, step;
  };
  Range range;
  static shared_ptr<CameraSetting> from(CameraWidget *widget, const shared_ptr<CameraSetting> &parent);
  shared_ptr<CameraSetting> find(const std::string &path);
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
  shared_ptr<CameraSetting> settings;
  uint64_t manualExposure = 0l;
    QString outputDirectory;
  void setting(const std::string &path, const QString &value);
  void reloadSettings();
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