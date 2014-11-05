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
class GPhoto : public ImagingDriver
{
  Q_OBJECT
private:
  class Private;
  friend class Private;
  std::shared_ptr<Private> const d;
  
public:
  class Camera : public Imager {
  public:
    Camera(const std::shared_ptr<Private> &d);
    ~Camera();
    virtual QString summary() const;
    virtual QString model() const;
    virtual QString about() const;
  private:
    std::shared_ptr<Private> const d;
  };
  friend class Camera;
    GPhoto(QObject *parent = 0);
    ~GPhoto();
    virtual std::shared_ptr<Imager> imager() const;
public slots:
  virtual void scan();
  virtual void preview();

};

#endif // GULINUX_GPHOTO_H
