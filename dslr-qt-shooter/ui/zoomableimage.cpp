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

ZoomableImage::~ZoomableImage()
{

}

ZoomableImage::ZoomableImage(QWidget* parent)
{
  setWidget(image = new QLabel);
  image->setBackgroundRole(QPalette::Base);
  image->setScaledContents(true);
  image->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  setWidgetResizable(true);
  setCursor(Qt::OpenHandCursor);
}

void ZoomableImage::fitToWindow()
{
  QPointF previousRatio = ratio();
  setWidgetResizable(true);
  image->adjustSize();
  scale_selection(previousRatio);
}

void ZoomableImage::normalSize()
{
  QPointF previousRatio = ratio();
    setWidgetResizable(false);
    image->adjustSize();
    _ratio = 1;
    scale_selection(previousRatio);
}

void ZoomableImage::mousePressEvent(QMouseEvent* event)
{
  if(selectionMode) {
    delete selection;
    selection = new QRubberBand(QRubberBand::Rectangle, image);
    selection->move(event->pos() + scrollPoint());

    return;
  }
    QAbstractScrollArea::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
      dragging = true;
      point = event->pos();
      QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    }
}

QPoint ZoomableImage::scrollPoint() const
{
  return {horizontalScrollBar()->value(), verticalScrollBar()->value()};
}


void ZoomableImage::startSelectionMode()
{
  selectionMode = true;
  setCursor(Qt::CrossCursor);
}


void ZoomableImage::mouseMoveEvent(QMouseEvent* e)
{
  if(selectionMode) {
    selection->setGeometry({selection->geometry().topLeft(), e->pos() + scrollPoint()});
    selection->show();
    return;
  }
  if(!dragging) return;
    QAbstractScrollArea::mouseMoveEvent(e);
  auto delta = point - e->pos();
  horizontalScrollBar()->setValue( horizontalScrollBar()->value() + delta.x());
  verticalScrollBar()->setValue(verticalScrollBar()->value() + delta.y());
  point = e->pos();
}

QPointF ZoomableImage::ratio() const
{
    double hratio = static_cast<double>(image->width()) / static_cast<double>(image->pixmap()->width());
    double vratio = static_cast<double>(image->height()) / static_cast<double>(image->pixmap()->height());
    return {hratio, vratio};
}

void ZoomableImage::mouseReleaseEvent(QMouseEvent* e)
{
  if(selectionMode) {
    double hratio = ratio().x();
    double vratio = ratio().y();
    selectionRect = { selection->geometry().x() / hratio,
		      selection->geometry().y() / vratio, 
		      selection->geometry().width() / hratio, 
		      selection->geometry().height() / vratio
      
    };
    qDebug() << "selection: " << selectionRect << ", widget size: " << image->size() << ", image size: " << image->pixmap()->size();
    qDebug() << "hscroll: " << horizontalScrollBar()->value() << ", vscroll: " << verticalScrollBar()->value();
    selectionMode = false;
    setCursor(Qt::OpenHandCursor);
    return;
  }
    QAbstractScrollArea::mouseReleaseEvent(e);
    dragging = false;
    QApplication::restoreOverrideCursor();
}




void ZoomableImage::scale(double factor)
{
  Q_ASSERT(image->pixmap());
  setWidgetResizable(false);

  QPointF previousRatio = ratio();
  _ratio *= factor;
  image->resize(_ratio * image->pixmap()->size());
  auto adjustScrollBar = [=](QScrollBar *scrollBar, double factor){
    scrollBar->setValue(int(factor * scrollBar->value() + ((factor - 1) * scrollBar->pageStep()/2)));
  };
  adjustScrollBar(horizontalScrollBar(), factor);
  adjustScrollBar(verticalScrollBar(), factor);
  scale_selection(previousRatio);
}

void ZoomableImage::scale_selection(QPointF previousRatio)
{
  if(!selection) return;
  
  QPointF _ratio = {ratio().x() / previousRatio.x(), ratio().y() / previousRatio.y()};
  QRect geometry = selection->geometry();
  selection->setGeometry(
    geometry.x() * _ratio.x(),
    geometry.y() * _ratio.y(),
    geometry.width() * _ratio.x(),
    geometry.height() * _ratio.y()
  );
}


void ZoomableImage::setImage(const QImage& image)
{
  this->image->setPixmap(QPixmap::fromImage(image));
}

QRect ZoomableImage::roi() const
{
  return selectionRect;
}

void ZoomableImage::clearROI()
{
  delete selection;
  selection = 0;
  selectionRect = {};
}

