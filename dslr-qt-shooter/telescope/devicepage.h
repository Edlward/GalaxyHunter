/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Marco Gulino <email>
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

#ifndef DEVICEPAGE_H
#define DEVICEPAGE_H

#include <QWidget>
#include <basedevice.h>
#include <memory>

class INDIClient;
class DevicePage : public QWidget
{
    Q_OBJECT

public:
    ~DevicePage();
    DevicePage(INDI::BaseDevice *device, const std::shared_ptr<INDIClient> &indiClient, QWidget* parent = 0);

private:
  class Private;
  friend class Private;
  std::unique_ptr<Private> d;
};

#endif // DEVICEPAGE_H
