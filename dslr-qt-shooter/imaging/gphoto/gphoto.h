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

#ifndef GULINUX_GPHOTO_H
#define GULINUX_GPHOTO_H

#include <memory>
#include <QObject>
#include "imaging_driver.h"

class QImage;
class GPhotoCameraInformation;
class GPhotoCamera;

class GPhoto : public ImagingDriver
{
  Q_OBJECT
public:
    GPhoto(QObject *parent = 0);
    ~GPhoto();
public slots:
protected:
  virtual void scan_imagers();

private:
  class Private;
  friend class Private;
  std::unique_ptr<Private> const d;
};

#endif // GULINUX_GPHOTO_H
