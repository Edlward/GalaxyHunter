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

#include "shootersettings.h"
#include <QSettings>
#include <QDebug>
#include <QStandardPaths>
#include "utils/qt.h"

#include "Qt/strings.h"
using namespace std;

class ShooterSettings::Private {
public:
  Private(QSettings &settings, ShooterSettings *q);
  QSettings &settings;
private:
  ShooterSettings *q;
};

ShooterSettings::Private::Private(QSettings& settings, ShooterSettings* q) : settings{settings}, q{q}
{
}

template<typename T>
void ShooterSettings_set(QSettings &settings, const QString& key, const T& value)
{
  settings.setValue(key, value);
}

template<typename T>
T ShooterSettings_get(QSettings &settings, const QString& key, const T& defaultValue = {})
{
  return qvariant_cast<T>(settings.value(key, defaultValue));
}


ShooterSettings::ShooterSettings(QSettings& settings) : dptr(settings, this)
{
}

ShooterSettings::~ShooterSettings()
{
}

#define setting(Name, Type, Default, ...) \
Type ShooterSettings:: __VA_ARGS__ Name () const { return ShooterSettings_get<Type>(d->settings, #Name, Default); } \
void ShooterSettings:: __VA_ARGS__ Name (Type value) { ShooterSettings_set<Type>(d->settings, #Name, value); }

#define setting_obj(Name, Type, Default, ...) \
Type ShooterSettings:: __VA_ARGS__ Name () const { return ShooterSettings_get<Type>(d->settings, #Name, Default); } \
void ShooterSettings:: __VA_ARGS__ Name (const Type &value) { ShooterSettings_set<Type>(d->settings, #Name, value); }

void ShooterSettings::shootMode(ShooterSettings::ShootMode mode)
{
  ShooterSettings_set<int>(d->settings, "shootMode", mode);
}

ShooterSettings::ShootMode ShooterSettings::shootMode() const
{
  return static_cast<ShootMode>(ShooterSettings_get<int>(d->settings, "shootMode", ShootMode::Single));
}

shared_ptr<ShooterSettings::Camera> ShooterSettings::camera(const ImagerPtr& imager, const Imager::Settings &settings)
{
  return CameraPtr{new Camera(imager->info().model, d->settings, settings)};
}

ShooterSettings::CameraPtr ShooterSettings::camera(const ImagerPtr& imager)
{
  return CameraPtr{new Camera(imager->info().model, d->settings)};
}




setting_obj(delayBetweenShots, QTime, QTime(0,0,0))
setting(ditherAfterEachShot, bool, false)
setting(saveImage, bool, false)
setting(sequenceLength, qint64, 0)
setting_obj(saveImageDirectory, QString, QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))



class ShooterSettings::Camera::Private {
public:
  Private(QSettings &settings, const Imager::Settings &imagerSettings, ShooterSettings::Camera *q);
  QSettings &settings;
  Imager::Settings imagerSettings;
private:
  ShooterSettings::Camera *q;
};

ShooterSettings::Camera::Private::Private(QSettings& settings, const Imager::Settings& imagerSettings, ShooterSettings::Camera* q) : settings(settings), imagerSettings(imagerSettings), q{q}
{
}

ShooterSettings::Camera::Camera(const QString name, QSettings& settings) : dptr(settings, {}, this)
{
  settings.beginGroup("camera_%1"_q % name);
}


ShooterSettings::Camera::Camera(const QString name, QSettings& settings, const Imager::Settings &imagerSettings) : dptr(settings, imagerSettings, this)
{
  qDebug() << "loading settings from default: " << imagerSettings;
  settings.beginGroup("camera_%1"_q % name);
  d->imagerSettings.imageFormat.current = d->settings.value("image_format", imagerSettings.imageFormat.current).toString();
  d->imagerSettings.iso.current = d->settings.value("iso", imagerSettings.iso.current).toString();
  d->imagerSettings.shutterSpeed.current = d->settings.value("shutter_speed", imagerSettings.shutterSpeed.current).toString();
  d->imagerSettings.manualExposure = d->settings.value("manual_exposure", imagerSettings.manualExposure).toBool();
  d->imagerSettings.manualExposureSeconds = d->settings.value("manual_exposure_seconds", imagerSettings.manualExposureSeconds).toLongLong();
  d->imagerSettings.serialShootPort = d->settings.value("serial_shoot_port", imagerSettings.serialShootPort).toString();
  qDebug() << "settings loaded: " << d->imagerSettings;
}

void ShooterSettings::Camera::save(const Imager::Settings& imagerSettings)
{
  d->imagerSettings = imagerSettings;
  d->settings.setValue("image_format", d->imagerSettings.imageFormat.current);
  d->settings.setValue("iso", d->imagerSettings.iso.current);
  d->settings.setValue("shutter_speed", d->imagerSettings.shutterSpeed.current);
  d->settings.setValue("manual_exposure", d->imagerSettings.manualExposure);
  d->settings.setValue("manual_exposure_seconds", d->imagerSettings.manualExposureSeconds);
  d->settings.setValue("serial_shoot_port", d->imagerSettings.serialShootPort);
  qDebug() << "saved: " << d->imagerSettings;
}

ShooterSettings::Camera::~Camera()
{
  d->settings.endGroup();
}

ShooterSettings::Camera::operator Imager::Settings() const
{
  return d->imagerSettings;
}



