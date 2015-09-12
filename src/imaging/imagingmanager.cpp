/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#include "imagingmanager.h"
#include "imagingsequence.h"
#include <QtConcurrent/QtConcurrent>
#include "commons/shootersettings.h"

using namespace std;
using namespace std::placeholders;

class ImagingManager::Private {
public:
  Private(ShooterSettings &shooterSettings, ImagingManager *q) : shooterSettings{shooterSettings}, q{q} {}
  ShooterSettings &shooterSettings;
  ImagerPtr imager;
  bool remove_on_camera;
  SequenceElement sequence;
  bool stopped = true;
private:
  ImagingManager *q;
};


ImagingManager::ImagingManager(ShooterSettings &shooterSettings, QObject* parent) : QObject(parent), dptr(shooterSettings, this)
{
}

ImagingManager::~ImagingManager()
{
}

void ImagingManager::setImager(const ImagerPtr& imager)
{
  d->imager = imager;
}

void ImagingManager::start(Sequence sequence)
{
  d->stopped = false;
  emit started();
  while(!sequence.isEmpty() && !d->stopped) {
    d->sequence = sequence.dequeue();
    auto imagingSequence = d->sequence.imagingSequence;
    if(imagingSequence) {
      imagingSequence->moveToThread(QThread::currentThread());
      connect(imagingSequence.get(), &ImagingSequence::started, this, &ImagingManager::started);
      connect(imagingSequence.get(), &ImagingSequence::finished, this, &ImagingManager::finished);
      connect(imagingSequence.get(), &ImagingSequence::aborted, this, &ImagingManager::finished); // todo
      connect(imagingSequence.get(), &ImagingSequence::image, bind(&ImagingManager::image, this, _1, _2));
      imagingSequence->start();
    }
    if(d->sequence.run_after_sequence)
      d->sequence.run_after_sequence();
    d->sequence = {};
  }
  emit finished();
  d->stopped = true;
}


void ImagingManager::abort()
{
  if(d->sequence.imagingSequence)
    d->sequence.imagingSequence->abort();
  d->stopped = true;
}


void ImagingManager::setExposure(double milliseconds)
{
}


void ImagingManager::setRemoveOnCameraEnabled(bool enabled)
{
  d->remove_on_camera = enabled;
}




#include "imagingmanager.moc"
