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
#include "Qt/strings.h"

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
  int item = 0;
  auto total = sequence.size();
  while(!sequence.isEmpty() && !d->stopped) {
    d->sequence = sequence.dequeue();
    emit message( d->sequence.displayName.isEmpty() ? tr("Started sequence") : tr("Started sequence %1").arg(d->sequence.displayName), 0);
    qDebug() << __PRETTY_FUNCTION__ << ": Running sequence item" << ++item << "out of" << total << "," << d->sequence.displayName;
    auto imagingSequence = d->sequence.imagingSequence;
    if(imagingSequence) {
      qDebug() <<__PRETTY_FUNCTION__ << " running imagingSequence";
//       imagingSequence->moveToThread(QThread::currentThread()); // TODO: not needed, also not working in pull mode
      connect(imagingSequence.get(), &ImagingSequence::started, this, &ImagingManager::started);
      connect(imagingSequence.get(), &ImagingSequence::finished, this, &ImagingManager::finished);
      connect(imagingSequence.get(), &ImagingSequence::aborted, this, &ImagingManager::finished); // todo
      connect(imagingSequence.get(), &ImagingSequence::image, bind(&ImagingManager::image, this, _1));
      auto total_images = imagingSequence->settings().shots;
      int shots = 0;
      connect(imagingSequence.get(), &ImagingSequence::image, [=, &shots]{
        shots++;
        emit message(imagingSequence->settings().mode == ShooterSettings::Continuous ?
          tr("Image %1 for sequence %2") % shots % d->sequence.displayName :
          tr("Image %1 of %2 for sequence %3") % shots % total_images % d->sequence.displayName,
          0);
      });
      imagingSequence->start();
    }
    if(d->sequence.run_after_sequence) {
      qDebug() << __PRETTY_FUNCTION__ << " running after_sequence";
      d->sequence.run_after_sequence();
    }
    d->sequence = {};
  }
  qDebug() << __PRETTY_FUNCTION__ << ": finished.";
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
