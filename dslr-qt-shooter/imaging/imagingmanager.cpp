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
#include <QtConcurrent/QtConcurrent>
#include "commons/shootersettings.h"

using namespace std;

class ImagingManager::Private {
public:
  Private(ShooterSettings &shooterSettings, ImagingManager *q) : shooterSettings{shooterSettings}, q{q} {}
  ShooterSettings &shooterSettings;
  ImagerPtr imager;
  bool remove_on_camera;
  shared_ptr<bool> abort;
  
  struct SequenceRun {
    int remaining_shots;
    int delay_milliseconds;
    ImagerPtr imager;
    ImagingManager *q;
    shared_ptr<bool> abort;
    bool save;
    const QString saveDirectory;
    Imager::Settings::ptr imagerSettings;
    void start();
  };
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

void ImagingManager::start()
{
  int sequenceLength = 1;
  if(d->shooterSettings.shootMode() == ShooterSettings::Repeat) {
    sequenceLength = d->shooterSettings.sequenceLength() == 0 ? std::numeric_limits<int>().max() : d->shooterSettings.sequenceLength();
  }
  auto millisecondsDelayBetweenShots = QTime{0,0,0}.secsTo(d->shooterSettings.delayBetweenShots()) * 1000;
  
  d->abort = make_shared<bool>(false);
  emit started();
  QtConcurrent::run([=]{
    Private::SequenceRun sequence{sequenceLength, millisecondsDelayBetweenShots, d->imager, this, d->abort, d->shooterSettings.saveImage(), d->shooterSettings.saveImageDirectory()};
    sequence.start();
  });
}

void ImagingManager::Private::SequenceRun::start()
{
  while(remaining_shots > 0 && ! *abort) {
    auto image = imager->shoot(imagerSettings);
    if(save)
      image->save(saveDirectory);
    emit q->image(image, --remaining_shots);
    if(remaining_shots>0)
      QThread::msleep(delay_milliseconds);
  }
  emit q->finished();
}


void ImagingManager::abort()
{
  *d->abort = true;
}


void ImagingManager::setExposure(double milliseconds)
{
}


void ImagingManager::setRemoveOnCameraEnabled(bool enabled)
{
  d->remove_on_camera = enabled;
}




#include "imagingmanager.moc"
