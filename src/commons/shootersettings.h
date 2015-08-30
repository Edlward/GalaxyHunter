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

#ifndef SHOOTERSETTINGS_H
#define SHOOTERSETTINGS_H
#include "dptr.h"
#include <imaging/imaging_driver.h>
#include <QString>
#include <QDateTime>

class QSettings;
class ShooterSettings
{
public:
    ShooterSettings(QSettings &settings);
    ~ShooterSettings();
    
    class Camera {
    public:
      ~Camera();
      operator Imager::Settings() const;
      void save(const Imager::Settings &imagerSettings);
      
    private:
      friend class ShooterSettings;
      Camera(const QString name, QSettings &settings);
      Camera(const QString name, QSettings &settings, const Imager::Settings &imagerSettings);
      D_PTR
    };
    
    typedef std::shared_ptr<Camera> CameraPtr;
    
    enum ShootMode { Single, Continuous, Sequence };
    
    ShootMode shootMode() const;
    void shootMode(ShootMode mode);
    
    int sequenceLength() const;
    void sequenceLength(int imagesCount);
    
    QTime delayBetweenShots() const;
    void delayBetweenShots(const QTime &delay);
    
    bool ditherAfterEachShot() const;
    void ditherAfterEachShot(bool dither);

    bool saveImage() const;
    void saveImage(bool save);
    
    QString saveImageDirectory() const;
    void saveImageDirectory(const QString &directory);
    
    CameraPtr camera(const ImagerPtr& imager, const Imager::Settings &settings);
    CameraPtr camera(const ImagerPtr& imager);
private:
  D_PTR
  friend class Camera;
};

#endif // SHOOTERSETTINGS_H
