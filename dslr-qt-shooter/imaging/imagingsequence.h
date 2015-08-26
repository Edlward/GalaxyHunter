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

#ifndef IMAGINGSEQUENCE_H
#define IMAGINGSEQUENCE_H

#include <QObject>
#include "dptr.h"
#include "imaging_driver.h"

class ImagingSequence : public QObject
{
public:
  struct SequenceSettings {
    std::size_t shots;
    long long delayBetweenShotsMilliseconds;
    bool clearPicturesFromCamera;
    bool saveToDisk;
    QString saveDirectory;
  };
  ImagingSequence(const ImagerPtr &imager, const Imager::Settings::ptr &ettings, const SequenceSettings &sequenceSettings, QObject* parent = 0);
public slots:
  void start();
  void abort();
signals:
  void finished();
  void aborted();
  void image(const Image::ptr &image, int remaining);
  
private:
    D_PTR
};

#endif // IMAGINGSEQUENCE_H
