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


#include "gphoto_commons.h"
#include "gphoto_camera.h"

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

