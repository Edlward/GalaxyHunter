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
#include "libgphoto++/src/driver.h"
using namespace std;

// sample application: http://sourceforge.net/p/gphoto/code/HEAD/tree/trunk/libgphoto2/examples/sample-multi-detect.c#l38
//                     http://sourceforge.net/p/gphoto/code/HEAD/tree/trunk/libgphoto2/examples/autodetect.c

class GPhoto::Private {
public:
  Private(ShooterSettings &shooterSettings, GPhoto *q);
  GPhotoCPP::DriverPtr driver;
  ShooterSettings &shooterSettings;
private:
  GPhoto *q;
};

GPhoto::Private::Private(ShooterSettings& shooterSettings, GPhoto* q) : driver{new GPhotoCPP::Driver /* TODO: logger */}, shooterSettings{shooterSettings}, q{q}
{

}


GPhoto::GPhoto(ShooterSettings &shooterSettings, QObject *parent) : ImagingDriver(parent), dptr(shooterSettings, this)
{
}


GPhoto::~GPhoto()
{
}


void GPhoto::scan_imagers()
{
  auto cameras = d->driver->cameras();
  for(auto camera: cameras) {
    qDebug() << "Found camera: " << QString::fromStdString( camera->name() );
    _imagers.push_back(make_shared<GPhotoCamera>(camera, d->shooterSettings));
  }
}

