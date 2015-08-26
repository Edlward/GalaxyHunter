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

#include "imagingsequence.h"
#include <QThread>

class ImagingSequence::Private {
public:
  Private(const ImagerPtr& imager, const Imager::Settings::ptr& imagerSettings, const ImagingSequence::SequenceSettings& sequenceSettings, ImagingSequence* q);
  ImagerPtr imager;
  Imager::Settings::ptr imagerSettings;
  SequenceSettings sequenceSettings;
  bool aborted = false;
private:
    ImagingSequence *q;
};

ImagingSequence::Private::Private(const ImagerPtr& imager, const Imager::Settings::ptr& imagerSettings, const ImagingSequence::SequenceSettings& sequenceSettings, ImagingSequence* q)
  : imager{imager}, imagerSettings{imagerSettings}, sequenceSettings{sequenceSettings}, q{q}
{
}



ImagingSequence::ImagingSequence(const ImagerPtr &imager, const Imager::Settings::ptr &imagerSettings, const SequenceSettings &sequenceSettings, QObject* parent)
    : dptr(imager, imagerSettings, sequenceSettings, this)
{
}

void ImagingSequence::start()
{
  while(d->sequenceSettings.shots > 0 && ! d->aborted) {
    auto image = d->imager->shoot(d->imagerSettings);
    if(d->sequenceSettings.saveToDisk)
      image->save(d->sequenceSettings.saveDirectory);
    emit this->image(image, --d->sequenceSettings.shots);
    if(d->sequenceSettings.shots>0)
      QThread::msleep(d->sequenceSettings.delayBetweenShotsMilliseconds);
  }
  if(d->aborted)
    emit aborted();
  else
    emit finished();
}

void ImagingSequence::abort()
{
  d->aborted = true;
}


