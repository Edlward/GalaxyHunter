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
#include <QScrollBar>

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
}

void ZoomableImage::fitToWindow()
{
  setWidgetResizable(true);
  image->adjustSize();
}

void ZoomableImage::normalSize()
{
    setWidgetResizable(false);
    image->adjustSize();
    ratio = 1;
}

void ZoomableImage::scale(double factor)
{
  Q_ASSERT(image->pixmap());
  setWidgetResizable(false);

  ratio *= factor;
  image->resize(ratio * image->pixmap()->size());
  auto adjustScrollBar = [=](QScrollBar *scrollBar, double factor){
    scrollBar->setValue(int(factor * scrollBar->value() + ((factor - 1) * scrollBar->pageStep()/2)));
  };
  adjustScrollBar(horizontalScrollBar(), factor);
  adjustScrollBar(verticalScrollBar(), factor);
}

void ZoomableImage::setImage(const QImage& image)
{
  this->image->setPixmap(QPixmap::fromImage(image));
}
