/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Marco Gulino <marco.gulino@bhuman.it>
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

#include "focus.h"
#include <QImage>
#include <QDebug>
#include "focus/fitshistogram.h"
#include "focus/fitsimage.h"
#include <Magick++.h>

Focus::~Focus()
{
}

Focus::Focus(QObject* parent)
{
}

void Focus::analyze(const QImage& image)
{
      // Focusing HFR calculation
      FITSImage fits_image;
      qDebug() << "Loading fit: " << fits_image.loadFITS(image);
      FITSHistogram histogram;
      histogram.setImage(&fits_image);
      histogram.constructHistogram(500, 500);
      fits_image.setHistogram(&histogram);
      qDebug() << "Stars: " << fits_image.findStars();
      auto hfr = fits_image.getHFR();
      qDebug() << "HFR: " << hfr;
      emit focus_rate(hfr*2, HFD);
}

#include "focus.moc"
