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

#include "zoomableimage.h"
#include <QLabel>
#include <QImage>
#include <QDragMoveEvent>
#include <QDrag>
#include <QPaintEngine>
#include <QScrollBar>
#include <QApplication>
#include <QDebug>
#include <QMimeData>
#include <QRubberBand>
#include <QGraphicsRectItem>

class ZoomableImage::Private {
public:
  Private(ZoomableImage *q) : q(q) {}
  ZoomableImage *q;
  QPoint moveOriginPoint;
  bool selectionMode = false;
  QRectF selectionRect;
  QGraphicsRectItem *selection = 0;
  QGraphicsScene scene;
  QRect imageSize;
};



ZoomableImage::~ZoomableImage()
{

}

ZoomableImage::ZoomableImage(QWidget* parent) : QGraphicsView(parent), d(new Private{this})
{
  setScene(&d->scene);
  setDragMode(QGraphicsView::ScrollHandDrag);
  connect(this, &QGraphicsView::rubberBandChanged, [=](QRect a,const QPointF &sceneStart, const QPointF &sceneEnd){
    if(!d->selectionMode || a.isEmpty())
      return;
    d->selectionRect = QRectF(sceneStart, sceneEnd);
  });
}

void ZoomableImage::fitToWindow()
{
  fitInView(d->imageSize, Qt::KeepAspectRatio);
}




void ZoomableImage::normalSize()
{
  setTransform({});
}



void ZoomableImage::scale(double factor)
{
  QGraphicsView::scale(factor, factor);
}




void ZoomableImage::startSelectionMode()
{
  clearROI();
  d->selectionMode = true;
  setDragMode(QGraphicsView::RubberBandDrag);
}

void ZoomableImage::mouseReleaseEvent(QMouseEvent* e)
{
  if(d->selectionMode) {
    d->selectionMode = false;
    d->selection = scene()->addRect(d->selectionRect, {Qt::green}, {QColor{0, 250, 250, 20}});
    qDebug() << "rect: " << d->selectionRect << ", " << d->selection->rect();
  }
  QGraphicsView::mouseReleaseEvent(e);
  setDragMode(QGraphicsView::ScrollHandDrag);
}



void ZoomableImage::setImage(const QImage& image)
{
  if(d->selection)
    d->scene.removeItem(d->selection);
  d->scene.clear();
  d->imageSize = image.rect();
  d->scene.addPixmap(QPixmap::fromImage(image));
  if(d->selection)
    d->scene.addItem(d->selection);
}

QRect ZoomableImage::roi() const
{
  return d->selectionRect.toRect();
}

void ZoomableImage::clearROI()
{
  scene()->removeItem(d->selection);
  delete d->selection;
  d->selection = 0;
  d->selectionRect = {};
}

