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
#include <QStandardPaths>

Q_DECLARE_METATYPE(ShooterSettings::ShootMode)

class ShooterSettings::Private {
public:
  Private(QSettings &settings, ShooterSettings *q);
  QSettings &settings;
  template<typename T> T get(const QString &key, const T &defaultValue = {}) const;
  template<typename T> void set(const QString &key, const T &value);
private:
  ShooterSettings *q;
};

ShooterSettings::Private::Private(QSettings& settings, ShooterSettings* q) : settings{settings}, q{q}
{
}

template<typename T>
void ShooterSettings::Private::set(const QString& key, const T& value)
{
  settings.setValue(key, value);
}

template<typename T>
T ShooterSettings::Private::get(const QString& key, const T& defaultValue) const
{
  return qvariant_cast<T>(settings.value(key, defaultValue));
}



ShooterSettings::ShooterSettings(QSettings& settings) : dptr(settings, this)
{
}

ShooterSettings::~ShooterSettings()
{
}

#define setting(Name, Type, Default) \
Type ShooterSettings:: Name () const { return d->get<Type>(#Name, Default); } \
void ShooterSettings:: Name (Type value) { d->set<Type>(#Name, value); }

#define setting_obj(Name, Type, Default) \
Type ShooterSettings:: Name () const { return d->get<Type>(#Name, Default); } \
void ShooterSettings:: Name (const Type &value) { d->set<Type>(#Name, value); }


setting(shootMode, ShooterSettings::ShootMode, ShooterSettings::Single)
setting_obj(delayBetweenShots, QTime, QTime(0,0,0))
setting(ditherAfterEachShot, bool, false)
setting(saveImage, bool, false)
setting(sequenceLength, int, 0)
setting_obj(saveImageDirectory, QString, QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))



