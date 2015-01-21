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
#include <QRect>
class QLabel;
class QImage;
class QRubberBand;
class ZoomableImage : public QScrollArea
{
Q_OBJECT
public:
    ~ZoomableImage();
    ZoomableImage(QWidget* parent);
  QRect roi() const;
public slots:
  void setImage(const QImage &image);
  void scale(double factor);
  void fitToWindow();
  void normalSize();
  void startSelectionMode();
  void clearROI();
protected:
    virtual void mousePressEvent(QMouseEvent*e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent*e);
private:
  QPoint point;
  QLabel *image;
  double _ratio = 1;
  bool dragging = false;
  bool selectionMode = false;
  QRect selectionRect;
  QRubberBand *selection = 0;
  QPoint scrollPoint() const;
  QPointF ratio() const;
  void scale_selection(QPointF previousRatio);
};

#endif // ZOOMABLEIMAGE_H
