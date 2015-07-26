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

#ifndef IMAGINGMANAGER_H
#define IMAGINGMANAGER_H

#include <QObject>

#include "utils/dptr.h"
#include "imaging_driver.h"

class ImagingManager : public QObject
{
    Q_OBJECT

public:
    ImagingManager(const ImagerPtr &imager, QObject *parent = 0);
    ~ImagingManager();
    enum Mode { Single, Sequence, Movie };
public slots:
  void start();
  void abort();
  void setMode(Mode mode);
  void setExposure(double milliseconds);
  void setFileName(const QString &file);
signals:
  void finished();
  void saved(const QString &file);
private:
  D_PTR
};

typedef std::shared_ptr<ImagingManager> ImagingManagerPtr;

#endif // IMAGINGMANAGER_H
