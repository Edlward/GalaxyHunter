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

class QImage;
class GPhoto : public QObject
{
  Q_OBJECT
private:
  class Private;
  friend class Private;
  std::shared_ptr<Private> const d;
  
public:
  class Camera {
  public:
    Camera(const std::shared_ptr<Private> &d);
    ~Camera();
    QString summary() const;
    QString model() const;
    QString about() const;
  private:
    std::shared_ptr<Private> const d;
  };
  friend class Camera;
    GPhoto(QObject *parent = 0);
    ~GPhoto();
    std::shared_ptr<Camera> camera() const;
public slots:
  void findCamera();
  void preview();
signals:
  void gphoto_message(const QString &);
  void gphoto_error(const QString &);
  void camera_connected();
  void camera_preview(const QImage &);
};

#endif // GULINUX_GPHOTO_H
