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
  shared_ptr<ImagingSequence> sequence;
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

void ImagingManager::start(const Imager::Settings::ptr &imagerSettings)
{
  int sequenceLength = 1;
  if(d->shooterSettings.shootMode() == ShooterSettings::Repeat) {
    sequenceLength = d->shooterSettings.sequenceLength() == 0 ? std::numeric_limits<int>().max() : d->shooterSettings.sequenceLength();
  }
  auto millisecondsDelayBetweenShots = QTime{0,0,0}.secsTo(d->shooterSettings.delayBetweenShots()) * 1000;
  
  ImagingSequence::SequenceSettings sequenceSettings{sequenceLength, millisecondsDelayBetweenShots, d->remove_on_camera, d->shooterSettings.saveImage(), d->shooterSettings.saveImageDirectory()};
  
  d->sequence = make_shared<ImagingSequence>(d->imager, imagerSettings, sequenceSettings);
  connect(d->sequence.get(), &ImagingSequence::started, this, &ImagingManager::started);
  connect(d->sequence.get(), &ImagingSequence::finished, this, &ImagingManager::finished);
  connect(d->sequence.get(), &ImagingSequence::aborted, this, &ImagingManager::finished); // todo
  connect(d->sequence.get(), &ImagingSequence::image, bind(&ImagingManager::image, this, _1, _2));
  d->sequence->start();
  d->sequence.reset();
}


void ImagingManager::abort()
{
  // TODO: extract sequence as a class
  if(d->sequence)
    d->sequence->abort();
}


void ImagingManager::setExposure(double milliseconds)
{
}


void ImagingManager::setRemoveOnCameraEnabled(bool enabled)
{
  d->remove_on_camera = enabled;
}




#include "imagingmanager.moc"
