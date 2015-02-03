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

#include "telescopecontrol.h"
#include "indiclient.h"
#include "devicespanel.h"
#include "telescoperemotecontrol.h"
#include <basedevice.h>

#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>

class TelescopeControl::Private {
public:
  Private(TelescopeControl *q);
  std::shared_ptr<INDIClient> indiClient;
private:
  TelescopeControl *q;
};

TelescopeControl::Private::Private(TelescopeControl* q) : q(q)
{
}

TelescopeControl::~TelescopeControl()
{
}

TelescopeControl::TelescopeControl(QObject* parent) : d(new Private(this))
{
  d->indiClient = std::make_shared<INDIClient>();
}

void TelescopeControl::open(QString address, int port)
{
  d->indiClient->open(address, port);
}

void TelescopeControl::showControlPanel()
{
  (new DevicesPanel(d->indiClient))->show();
}

void TelescopeControl::showTelescopeRemoteControl()
{
  auto telescopes = d->indiClient->telescopes();
  if(telescopes.size() == 0) {
    QMessageBox::warning(nullptr, tr("No Telescopes Found"), tr("No telescopes found by INDI client. Try connecting to a server, and clicking 'connect' on the telescope you want to use, and retry"));
    return;
  }
  QStringList telescopeNames;
  std::transform(begin(telescopes), end(telescopes), std::back_inserter(telescopeNames), [](INDI::BaseDevice *dev) { return QString{dev->getDeviceName()}; } );
  QString telescopeName = QInputDialog::getItem(nullptr, tr("Telescope"), tr("Choose telescope"), telescopeNames, 0, false);
  if(telescopeName.length() == 0)
    return;
  auto telescope = telescopes[telescopeNames.indexOf(telescopeName)];
  (new TelescopeRemoteControl{d->indiClient, telescope})->show();
}

#include "telescopecontrol.moc"
