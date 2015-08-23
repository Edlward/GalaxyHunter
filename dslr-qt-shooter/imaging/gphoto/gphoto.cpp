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
#include <QThread>
#include "utils/scope.h"

#include "commons/shootersettings.h"
#include "gphoto_commons.h"
#include "gphoto_camera.h"

using namespace std;

// sample application: http://sourceforge.net/p/gphoto/code/HEAD/tree/trunk/libgphoto2/examples/sample-multi-detect.c#l38
//                     http://sourceforge.net/p/gphoto/code/HEAD/tree/trunk/libgphoto2/examples/autodetect.c

class GPhoto::Private {
public:
  Private(ShooterSettings &shooterSettings, GPhoto *q);
  GPContext* context;
  string last_error;
  string last_message;
  static void gphotoMessage(GPContext *context, const char *, void *);
  static void gphotoErrorMessage(GPContext *context, const char *, void *);
  void reset_messages();
  QMutex mutex;
  ShooterSettings &shooterSettings;
private:
  GPhoto *q;
};

GPhoto::Private::Private(ShooterSettings& shooterSettings, GPhoto* q) : shooterSettings{shooterSettings}, q{q}
{

}


GPhoto::GPhoto(ShooterSettings &shooterSettings, QObject *parent) : ImagingDriver(parent), dptr(shooterSettings, this)
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
  size_t cameras_size;
  gp_list_new(&cameras);
  list<shared_ptr<GPhotoCameraInformation>> cameras_info;
  gp_api{{
    sequence_run([&]{ return gp_camera_autodetect(cameras, d->context); }),
    sequence_run([&]{  cameras_size = gp_list_count(cameras); return static_cast<int>(cameras_size); }),
    sequence_run([&]{  
      for(int i=0; i<cameras_size; i++) {
	const char *name, *port;
	gp_list_get_name(cameras, i, &name);
	gp_list_get_value(cameras, i, &port);
	qDebug() << "found camera " << i << ": " << name << ", port " << port;
	cameras_info.push_back(make_shared<GPhotoCameraInformation>(name, port, d->context, d->mutex));
	return GP_OK;
      }
    }),
  }, make_shared<QMutexLocker>(&d->mutex)};
  gp_list_unref(cameras);
  for(auto camera: cameras_info)
    _imagers.push_back(make_shared<GPhotoCamera>(camera, d->shooterSettings));
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

