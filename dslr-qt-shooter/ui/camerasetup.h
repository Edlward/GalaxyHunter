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

#ifndef CAMERASETUP_H
#define CAMERASETUP_H

#include <QWidget>
#include "dptr.h"
#include <imaging/imaging_driver.h>

class ImagingSequence;
class ShooterSettings;
class CameraSetup : public QWidget
{
    Q_OBJECT

public:
    ~CameraSetup();
    CameraSetup(ShooterSettings &shooterSettings, QWidget* parent = 0);
    Imager::Settings::ptr imagerSettings() const;
    std::shared_ptr<ImagingSequence> imagingSequence() const;
public slots:
  void shooting(bool isShooting);
  void setCamera(const ImagerPtr &imager);
private:
  D_PTR
};

#endif // CAMERASETUP_H