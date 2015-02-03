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

#ifndef TELESCOPEREMOTECONTROL_H
#define TELESCOPEREMOTECONTROL_H

#include <QDialog>
#include <memory>
class INDIClient;
namespace INDI { class BaseDevice; }
namespace Ui
{
class TelescopeRemoteControl;
}

class TelescopeRemoteControl : public QDialog
{
    Q_OBJECT
public:
    ~TelescopeRemoteControl();
    TelescopeRemoteControl(const std::shared_ptr<INDIClient> &client, INDI::BaseDevice *device, QWidget* parent = 0);
private:
    Ui::TelescopeRemoteControl* ui;
    std::shared_ptr<INDIClient> client;
    INDI::BaseDevice *device;
};

#endif // TELESCOPEREMOTECONTROL_H
