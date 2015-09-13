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
#include <QDateTime>
#include "dptr.h"
#include "imaging_driver.h"
#include <commons/shootersettings.h>

class ImagingSequence : public QObject
{
  Q_OBJECT
public:
  struct SequenceSettings {
    ShooterSettings::ShootMode mode;
    qint64 shots;
    QTime delayBetweenShots;
    bool clearPicturesFromCamera;
    bool saveToDisk;
    QString saveDirectory;
    bool ditherAfterShots;
    
    operator bool() const;
    qint64 operator--();
    qint64 delayInMilliseconds() const;
  };
  ImagingSequence(const ImagerPtr& imager, const Imager::Settings& imagerSettings, const ImagingSequence::SequenceSettings& sequenceSettings, QObject* parent = 0);
  SequenceSettings settings() const;
  Imager::Settings imagerSettings() const;
  virtual ~ImagingSequence();
  typedef std::shared_ptr<ImagingSequence> ptr;
public slots:
  void start();
  void abort();
private:
    D_PTR
  
signals:
  void started();
  void finished();
  void aborted();
  void image(const Image::ptr &image);
  void dither();
};
#endif // IMAGINGSEQUENCE_H
