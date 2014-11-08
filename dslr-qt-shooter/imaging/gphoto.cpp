/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Marco Gulino <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "gphoto.h"
#include <gphoto2/gphoto2.h>
#include <sstream>
#include <iostream>
#include <QDebug>
#include <QTemporaryFile>
#include <QImage>
#include "utils/scope.h"
#include "utils/sequence.h"

#include <GraphicsMagick/Magick++.h>
#include <boost/algorithm/string.hpp>

using namespace std;

// sample application: http://sourceforge.net/p/gphoto/code/HEAD/tree/trunk/libgphoto2/examples/sample-multi-detect.c#l38
//                     http://sourceforge.net/p/gphoto/code/HEAD/tree/trunk/libgphoto2/examples/autodetect.c
typedef sequence<int, GP_OK, std::greater_equal<int>> gp_api;

class GPhoto::Private {
public:
  Private() {}
  GPContext* context;
  string last_error;
  string last_message;
  static void gphotoMessage(GPContext *context, const char *, void *);
  static void gphotoErrorMessage(GPContext *context, const char *, void *);
  void reset_messages();
};


class GPhotoCameraInformation {
public:
  GPhotoCameraInformation(const std::string &name, const std::string &port, GPContext *context) 
    : name(name), port(port), context(context) {}
  string name;
  string port;
  GPContext *context;
};

class GPhotoCamera::Private {
public:
  Private(const std::shared_ptr<GPhotoCameraInformation> &info)
    : model(QString::fromStdString(info->name)), port(info->port), context(info->context) {}
  string port;
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



GPhoto::GPhoto(QObject *parent) : ImagingDriver(parent), d(new Private)
{
  d->context = gp_context_new();
  gp_context_set_error_func(d->context, reinterpret_cast<GPContextErrorFunc>(&Private::gphotoErrorMessage), d.get());
  gp_context_set_message_func(d->context, reinterpret_cast<GPContextErrorFunc>(&Private::gphotoMessage), d.get());
}


GPhoto::~GPhoto()
{
  gp_context_unref(d->context);
}


void GPhoto::scan_imagers()
{
  CameraList *cameras;
  gp_list_new(&cameras);
  scope cleanup{[=]{ gp_list_unref(cameras); }};
  int result = gp_camera_autodetect(cameras, d->context);
  size_t cameras_size = gp_list_count(cameras);
  qDebug() << "result: " << result << ", list size: " << cameras_size;
  for(int i=0; i<cameras_size; i++) {
    const char *name, *port;
    gp_list_get_name(cameras, i, &name);
    gp_list_get_value(cameras, i, &port);
    qDebug() << "found camera " << i << ": " << name << ", port " << port;
    _imagers.push_back(make_shared<GPhotoCamera>(make_shared<GPhotoCameraInformation>(name, port, d->context)));
  }
}

void GPhoto::Private::gphotoMessage(GPContext* context, const char* m, void* data)
{
  Private *me = reinterpret_cast<Private*>(data);
  me->last_message = {m};
}

void GPhoto::Private::gphotoErrorMessage(GPContext* context, const char* m, void* data)
{
  Private *me = reinterpret_cast<Private*>(data);
  string message(m);
  me->last_error = message;
}

void GPhoto::Private::reset_messages()
{
  last_error.clear();
  last_message.clear();
}



GPhotoCamera::GPhotoCamera(const shared_ptr< GPhotoCameraInformation > &gphotoCameraInformation)
  : d(new Private{gphotoCameraInformation})
{
  gp_api{{
    { [=] { return gp_camera_new(&d->camera); } },
  }}.on_error([=](int errorCode, const std::string &label) {
    const char *errorMessage = gp_result_as_string(errorCode);
    qDebug() << errorMessage;
    // TODO: error signal? exception?
  });
  /*
  if(gp_camera_init(d->gphoto_camera, d->context) != GP_OK)
    throw runtime_error(d->last_error);
  
  gp_camera_get_abilities (d->gphoto_camera, &d->abilities);
  d->model = QString::fromLocal8Bit(d->abilities.model);
  gp_camera_get_summary(d->gphoto_camera, &d->camera_text, d->context);
  d->summary = QString::fromLocal8Bit(d->camera_text.text);
  gp_camera_get_about(d->gphoto_camera, &d->camera_text, d->context);
  d->about = QString::fromLocal8Bit(d->camera_text.text);
  */
}

void GPhotoCamera::connect()
{
  CameraAbilities abilities;
  GPPortInfo portInfo;
  CameraAbilitiesList *abilities_list = nullptr;
  GPPortInfoList *portInfoList = nullptr;
  int model, port;
  gp_api{{
    sequence_run( [&]{ return gp_abilities_list_new (&abilities_list); } ),
    sequence_run( [&]{ return gp_abilities_list_load(abilities_list, d->context); } ),
    sequence_run( [&]{ model = gp_abilities_list_lookup_model(abilities_list, d->model.toLocal8Bit()); return model; } ),
    sequence_run( [&]{ return gp_abilities_list_get_abilities(abilities_list, model, &abilities); } ),
    sequence_run( [&]{ return gp_camera_set_abilities(d->camera, abilities); } ),
    sequence_run( [&]{ return gp_port_info_list_new(&portInfoList); } ),
    sequence_run( [&]{ return gp_port_info_list_load(portInfoList); } ),
    sequence_run( [&]{ return gp_port_info_list_count(portInfoList); } ),
    sequence_run( [&]{ port = gp_port_info_list_lookup_path(portInfoList, d->port.c_str()); return port; } ),
    sequence_run( [&]{ return gp_port_info_list_get_info(portInfoList, port, &portInfo); return port; } ),
    sequence_run( [&]{ return gp_camera_set_port_info(d->camera, portInfo); } ),
    sequence_run( [&]{ emit connected(); return GP_OK; } ),
  }}.on_error([=](int errorCode, const std::string &label) {
    const char *errorMessage = gp_result_as_string(errorCode);
    qDebug() << "on " << QString::fromStdString(label) << ": " << errorMessage;
    emit error(this, QString::fromLocal8Bit(errorMessage));
  });
  gp_port_info_list_free(portInfoList);
  gp_abilities_list_free(abilities_list);
}

void GPhotoCamera::disconnect()
{
  gp_camera_exit(d->camera, d->context);
}

void GPhotoCamera::shoot()
{
  bool delete_file = true;
  CameraTempFile camera_file;
  CameraFilePath camera_remote_file;
  auto fixed_filename = [](const std::string &filename) { return boost::replace_all_copy(filename, "*", ""); };
  QImage image;
  gp_api{{
    sequence_run( [&]{ return gp_camera_capture(d->camera, GP_CAPTURE_IMAGE, &camera_remote_file, d->context);} ),
    sequence_run( [&]{ return gp_camera_file_get(d->camera, camera_remote_file.folder, fixed_filename(camera_remote_file.name).c_str(), GP_FILE_TYPE_NORMAL, camera_file, d->context); } ),
    sequence_run( [&]{ return camera_file.save();} ),
    sequence_run( [&]{ if(!delete_file) return GP_OK; return gp_camera_file_delete(d->camera, camera_remote_file.folder, fixed_filename(camera_remote_file.name).c_str(), d->context);} ),
  }}.run_last([&]{
    qDebug() << "shoot completed: camera file " << camera_file.path();
    if(image.load(camera_file)) {
      emit preview(image);
      return;
    }
    qDebug() << "Unable to load image; trying to convert it using GraphicsMagick.";
    Magick::Image m_image;
    m_image.read(camera_file.path().toStdString());
    Magick::Blob blob;
    m_image.write(&blob, "PNG");
    QByteArray data(static_cast<int>(blob.length()), 0);
    std::copy(reinterpret_cast<const char*>(blob.data()), reinterpret_cast<const char*>(blob.data()) + blob.length(), begin(data));
    if(image.loadFromData(data)) {
      emit preview(image);
      emit message(this, "image captured correctly");
      return;
    }
    qDebug() << "Error loading image.";
    emit error(this, "Error loading image");
  }).on_error([=](int errorCode, const std::string &label) {
    const char *errorMessage = gp_result_as_string(errorCode);
    qDebug() << "on " << QString::fromStdString(label) << ": " << errorMessage << "(" << errorCode << ")";
    emit error(this, QString::fromLocal8Bit(errorMessage));
  });
}

QString GPhotoCamera::about() const
{
  return d->about;
}

QString GPhotoCamera::model() const
{
  return d->model;
}

QString GPhotoCamera::summary() const
{
  return d->summary;
}


GPhotoCamera::~GPhotoCamera()
{
  disconnect(); // TODO: check if connected
  gp_camera_free(d->camera);
}




CameraTempFile::CameraTempFile()
{
  int r = gp_file_new(&camera_file);
  qDebug() << __PRETTY_FUNCTION__ << ": gp_file_new=" << r;
  temp_file.open();
  temp_file.close();
  temp_file.setAutoRemove(false);
}

int CameraTempFile::save()
{
  qDebug() << __PRETTY_FUNCTION__;
  return gp_file_save(camera_file, temp_file.fileName().toLocal8Bit());
}

CameraTempFile::~CameraTempFile()
{
  gp_file_free(camera_file);
}

QString CameraTempFile::mimeType() const
{
  int r = gp_file_detect_mime_type(camera_file);
  qDebug() << __PRETTY_FUNCTION__ << ": gp_file_detect_mime_type=" << r;
  const char *mime;
  r = gp_file_get_mime_type(camera_file, &mime);
  qDebug() << __PRETTY_FUNCTION__ << ": gp_file_get_mime_type=" << r;
  return QString(mime);
}

/*
void GPhoto::preview()
{
  CameraTempFile camera_file;
  int result = gp_camera_capture_preview(d->gphoto_camera, camera_file, d->context);
  camera_file.save();
  qDebug() << "result: " << result << ", file: " << camera_file.path() << ", mime: " << camera_file.mimeType();

  QImage image;
  qDebug() << "preview loaded: " << image.load(camera_file);
  emit camera_preview(image);
}

*/
