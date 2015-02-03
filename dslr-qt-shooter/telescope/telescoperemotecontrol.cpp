/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Marco Gulino <marco.gulino@bhuman.it>
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

#include "telescoperemotecontrol.h"
#include "indiclient.h"
#include "switchvectorproperty.h"
#include "numbervectorproperty.h"
#include "ui_telescoperemotecontrol.h"
#include <basedevice.h>

TelescopeRemoteControl::~TelescopeRemoteControl()
{
    delete ui;
}

TelescopeRemoteControl::TelescopeRemoteControl(const std::shared_ptr<INDIClient> &client, INDI::BaseDevice *device, QWidget* parent)
  : QDialog(parent), client(client), device(device), db(QSqlDatabase::addDatabase("QSQLITE"))
{
    db.setDatabaseName(OBJECTS_DATABASE);
    ui = new Ui::TelescopeRemoteControl;
    ui->setupUi(this);
    QVBoxLayout *manualMoveLayout = new QVBoxLayout;
    ui->manualMove->setLayout(manualMoveLayout);
    QHBoxLayout *controlsLayout = new QHBoxLayout;
    manualMoveLayout->addLayout(controlsLayout);
    
    controlsLayout->addWidget(new SwitchVectorProperty(device->getProperty("TELESCOPE_MOTION_NS", INDI_SWITCH)->getSwitch(), client));
    controlsLayout->addWidget(new SwitchVectorProperty(device->getProperty("TELESCOPE_MOTION_WE", INDI_SWITCH)->getSwitch(), client));
    controlsLayout->addWidget(new SwitchVectorProperty(device->getProperty("TELESCOPE_ABORT_MOTION", INDI_SWITCH)->getSwitch(), client));
    manualMoveLayout->addWidget(new SwitchVectorProperty(device->getProperty("SLEWMODE", INDI_SWITCH)->getSwitch(), client));
    QVBoxLayout *coordinatesLayout = new QVBoxLayout;
    ui->coordinates->setLayout(coordinatesLayout);
    coordinatesLayout->addWidget(new NumberVectorProperty(device->getProperty("EQUATORIAL_EOD_COORD", INDI_NUMBER)->getNumber(), client));
}
