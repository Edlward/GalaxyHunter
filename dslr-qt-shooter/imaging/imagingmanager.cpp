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

class ImagingManager::Private {
public:
  Private(ImagingManager *q) : q{q} {}
  ImagerPtr imager;
  bool remove_on_camera;
  bool save_enabled;
  QString outputDirectory;
  
  struct SequenceRun {
    int remaining_shots;
    double delay_milliseconds;
    ImagerPtr imager;
    ImagingManager *q;
    void start();
  };
private:
  ImagingManager *q;
};


ImagingManager::ImagingManager(QObject* parent) : QObject(parent), dpointer(this)
{
}

ImagingManager::~ImagingManager()
{
}

void ImagingManager::setImager(const ImagerPtr& imager)
{
  d->imager = imager;
}

void ImagingManager::start(int numberOfShots, double millisecondsDelayBetweenShots)
{
  emit started();
  QtConcurrent::run([=]{
    Private::SequenceRun sequence{numberOfShots, millisecondsDelayBetweenShots, d->imager, this};
    sequence.start();
  });
}

void ImagingManager::Private::SequenceRun::start()
{
  while(remaining_shots > 0) {
    auto image = imager->shoot();
    emit q->image(image, --remaining_shots);
    if(remaining_shots>0)
      QThread::msleep(delay_milliseconds);
  }
  emit q->finished();
}


void ImagingManager::abort()
{

}


void ImagingManager::setExposure(double milliseconds)
{
}



void ImagingManager::setOutputDirectory(const QString& directory)
{
  d->outputDirectory = directory;
}

void ImagingManager::setRemoveOnCameraEnabled(bool enabled)
{
  d->remove_on_camera = enabled;
}

void ImagingManager::setSaveEnabled(bool enabled)
{
  d->save_enabled = enabled;
}


#include "imagingmanager.moc"
