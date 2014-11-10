#ifndef GULINUX_GPHOTO_CAMERA_P_H
#define GULINUX_GPHOTO_CAMERA_P_H

#include "gphoto_camera.h"
#include "gphoto_commons.h"
#include <QTemporaryFile>
#include "utils/scope.h"
#include <QThread>
#include <QImage>
#include <QDebug>

#include <GraphicsMagick/Magick++.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>


using namespace std;
class GPhotoCamera::Private {
public:
  Private(const std::shared_ptr<GPhotoCameraInformation> &info)
    : model(QString::fromStdString(info->name)), port(info->port), context(info->context) {}
  std::string port;
  QString model;
  QString about;
  QString summary;
  GPContext* context;
  Camera *camera = nullptr;
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


struct CameraSetting : enable_shared_from_this<CameraSetting> {
  int id;
  std::string name;
  std::string label;
  std::string info;
  CameraWidgetType type;
  std::string path() const;
  std::vector<shared_ptr<CameraSetting>> children;
  std::shared_ptr<CameraSetting> parent;
  static std::shared_ptr<CameraSetting> from(CameraWidget *widget, const std::shared_ptr<CameraSetting> &parent);
};

#endif