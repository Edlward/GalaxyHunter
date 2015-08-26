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

#ifndef IMAGING_SEQUENCE_H
#define IMAGING_SEQUENCE_H

#include <QObject>
#include "dptr.h"
#include "imaging_driver.h"

class ImagingSequence : public QObject
{
  Q_OBJECT
public:
  struct SequenceSettings {
    std::size_t shots;
    long long delayBetweenShotsMilliseconds;
    bool clearPicturesFromCamera;
    bool saveToDisk;
    QString saveDirectory;
  };
  ImagingSequence(const ImagerPtr& imager, const Imager::Settings::ptr& imagerSettings, const ImagingSequence::SequenceSettings& sequenceSettings, QObject* parent = 0);
  virtual ~ImagingSequence();
public slots:
  void start();
  void abort();
private:
    D_PTR
  
signals:
  void started();
  void finished();
  void aborted();
  void image(const Image::ptr &image, int remaining);
};

#endif // IMAGINGSEQUENCE_H
