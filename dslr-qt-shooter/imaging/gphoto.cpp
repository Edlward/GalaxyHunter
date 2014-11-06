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

using namespace std;

// sample application: http://sourceforge.net/p/gphoto/code/HEAD/tree/trunk/libgphoto2/examples/sample-multi-detect.c#l38
//                     http://sourceforge.net/p/gphoto/code/HEAD/tree/trunk/libgphoto2/examples/autodetect.c

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
    : name(QString::fromStdString(info->name)), port(info->port), context(info->context) {}
  QString name;
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
  void save();
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


void GPhoto::scan()
{
  _imagers.clear();
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
  emit scan_finished();
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
  gp_camera_new(&d->camera);
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
  sequence<int, 0> sequence{{}};
  gp_camera_init(d->camera, d->context);
}

void GPhotoCamera::disconnect()
{
}

void GPhotoCamera::shootPreview()
{

}

QString GPhotoCamera::name() const
{
  return d->name;
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
  gp_camera_free(d->camera);
}




CameraTempFile::CameraTempFile()
{
  gp_file_new(&camera_file);
  temp_file.open();
  temp_file.close();
  temp_file.setAutoRemove(false);
}

void CameraTempFile::save()
{
  gp_file_save(camera_file, temp_file.fileName().toLocal8Bit());
}

CameraTempFile::~CameraTempFile()
{
  gp_file_free(camera_file);
}

QString CameraTempFile::mimeType() const
{
  gp_file_detect_mime_type(camera_file);
  const char *mime;
  gp_file_get_mime_type(camera_file, &mime);
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