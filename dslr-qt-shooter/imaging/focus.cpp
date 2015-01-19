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

Focus::Focus(int history, QObject* parent) : history_size(history), QObject(parent)
{
}

void Focus::analyze(const QImage& image)
{
      // Focusing HFR calculation
      FITSImage fits_image;
      if(!fits_image.loadFITS(image)) {
	qDebug() << "Error loading image for HFR analysis";
	return;
      }
      FITSHistogram histogram;
      histogram.setImage(&fits_image);
      histogram.constructHistogram(500, 500);
      fits_image.setHistogram(&histogram);
      if(!fits_image.findStars() > 0) {
	qDebug() << "HFR Analysis: error finding stars!";
	return;
      }
      auto hfd = fits_image.getHFR()*2;
      _history.push_front(hfd);
      while(_history.size() > history_size)
	_history.removeLast();
      emit focus_rate(hfd);
}

#include "focus.moc"
