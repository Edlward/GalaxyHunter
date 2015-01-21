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


class ZoomableImage::Private {
public:
  Private(ZoomableImage *q) : q(q) {}
  const ZoomableImage *q;
  QPoint moveOriginPoint;
  QLabel *imageWidget;
  double _ratio = 1;
  bool dragging = false;
  bool selectionMode = false;
  QRect selectionRect;
  QRubberBand *selection = 0;
  QPoint scrollPoint() const;
  QPointF ratio() const;
  void scale_selection(QPointF previousRatio);
  double image_ratio() const;
};


QPoint ZoomableImage::Private::scrollPoint() const
{
  return {q->horizontalScrollBar()->value(), q->verticalScrollBar()->value()};
}

QPointF ZoomableImage::Private::ratio() const
{
    double hratio = static_cast<double>(imageWidget->width()) / static_cast<double>(imageWidget->pixmap()->width());
    double vratio = static_cast<double>(imageWidget->height()) / static_cast<double>(imageWidget->pixmap()->height());
    return {hratio, vratio};
}

void ZoomableImage::Private::scale_selection(QPointF previousRatio)
{
  if(!selection) return;
  
  QPointF _ratio = {ratio().x() / previousRatio.x(),ratio().y() / previousRatio.y()};
  QRect geometry = selection->geometry();
  selection->setGeometry(
    geometry.x() * _ratio.x(),
    geometry.y() * _ratio.y(),
    geometry.width() * _ratio.x(),
    geometry.height() * _ratio.y()
  );
}

double ZoomableImage::Private::image_ratio() const
{
  return 
    static_cast<double>(imageWidget->pixmap()->width()) /
    static_cast<double>(imageWidget->pixmap()->height());
}



ZoomableImage::~ZoomableImage()
{

}

ZoomableImage::ZoomableImage(QWidget* parent) : d(new Private{this})
{
  setWidget(d->imageWidget = new QLabel);
  d->imageWidget->setBackgroundRole(QPalette::Base);
  d->imageWidget->setScaledContents(true);
  d->imageWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  setWidgetResizable(true);
  setCursor(Qt::OpenHandCursor);
}

void ZoomableImage::fitToWindow()
{
  QPointF previousRatio = d->ratio();
  setWidgetResizable(true);
  d->imageWidget->adjustSize();
  QSize imageSize = d->imageWidget->pixmap()->size();
  QSize widgetSize = d->imageWidget->size();
//   if(imageSize.width() > imageSize.height()) {
//     widgetSize.setHeight(widgetSize.width()/d->image_ratio() );
//   } else {
//     widgetSize.setWidth(widgetSize.height() * d->image_ratio());
//   }
//   setWidgetResizable(false);
//   d->imageWidget->resize(widgetSize);
//   d->scale_selection(previousRatio);
}

void ZoomableImage::normalSize()
{
  QPointF previousRatio = d->ratio();
    setWidgetResizable(false);
    d->imageWidget->adjustSize();
    d->_ratio = 1;
    d->scale_selection(previousRatio);
}



void ZoomableImage::scale(double factor)
{
  Q_ASSERT(d->imageWidget->pixmap());
  setWidgetResizable(false);

  QPointF previousRatio = d->ratio();
  d->_ratio *= factor;
  d->imageWidget->resize(d->_ratio * d->imageWidget->pixmap()->size());
  auto adjustScrollBar = [=](QScrollBar *scrollBar, double factor){
    scrollBar->setValue(int(factor * scrollBar->value() + ((factor - 1) * scrollBar->pageStep()/2)));
  };
  adjustScrollBar(horizontalScrollBar(), factor);
  adjustScrollBar(verticalScrollBar(), factor);
  d->scale_selection(previousRatio);
}


void ZoomableImage::mousePressEvent(QMouseEvent* event)
{
  if(d->selectionMode) {
    delete d->selection;
    d->selection = new QRubberBand(QRubberBand::Rectangle, d->imageWidget);
    d->selection->move(event->pos() + d->scrollPoint());

    return;
  }
    QAbstractScrollArea::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
      d->dragging = true;
      d->moveOriginPoint = event->pos();
      QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    }
}


void ZoomableImage::startSelectionMode()
{
  d->selectionMode = true;
  setCursor(Qt::CrossCursor);
}


void ZoomableImage::mouseMoveEvent(QMouseEvent* e)
{
  if(d->selectionMode) {
    d->selection->setGeometry({d->selection->geometry().topLeft(), e->pos() + d->scrollPoint()});
    d->selection->show();
    return;
  }
  if(!d->dragging) return;
    QAbstractScrollArea::mouseMoveEvent(e);
  auto delta = d->moveOriginPoint - e->pos();
  horizontalScrollBar()->setValue( horizontalScrollBar()->value() + delta.x());
  verticalScrollBar()->setValue(verticalScrollBar()->value() + delta.y());
  d->moveOriginPoint = e->pos();
}


void ZoomableImage::mouseReleaseEvent(QMouseEvent* e)
{
  if(d->selectionMode) {
    double hratio = d->ratio().x();
    double vratio = d->ratio().y();
    d->selectionRect = { d->selection->geometry().x() / hratio,
		      d->selection->geometry().y() / vratio, 
		      d->selection->geometry().width() / hratio, 
		      d->selection->geometry().height() / vratio
      
    };
    qDebug() << "selection: " << d->selectionRect << ", widget size: " << d->imageWidget->size() << ", image size: " << d->imageWidget->pixmap()->size();
    qDebug() << "hscroll: " << horizontalScrollBar()->value() << ", vscroll: " << verticalScrollBar()->value();
    d->selectionMode = false;
    setCursor(Qt::OpenHandCursor);
    return;
  }
    QAbstractScrollArea::mouseReleaseEvent(e);
    d->dragging = false;
    QApplication::restoreOverrideCursor();
}





void ZoomableImage::setImage(const QImage& image)
{
  this->d->imageWidget->setPixmap(QPixmap::fromImage(image));
}

QRect ZoomableImage::roi() const
{
  return d->selectionRect;
}

void ZoomableImage::clearROI()
{
  delete d->selection;
  d->selection = 0;
  d->selectionRect = {};
}

