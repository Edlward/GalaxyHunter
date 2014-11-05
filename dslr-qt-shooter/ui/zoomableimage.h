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

#ifndef ZOOMABLEIMAGE_H
#define ZOOMABLEIMAGE_H

#include <QScrollArea>

class QLabel;
class QImage;
class ZoomableImage : public QScrollArea
{
Q_OBJECT
public:
    ~ZoomableImage();
    ZoomableImage(QWidget* parent);
public slots:
  void setImage(const QImage &image);
  void scale(double factor);
  void fitToWindow();
  void normalSize();
protected:
    virtual void mousePressEvent(QMouseEvent*e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent*e);
private:
  QPoint point;
  QLabel *image;
  double ratio = 1;
  bool dragging = false;
};

#endif // ZOOMABLEIMAGE_H
