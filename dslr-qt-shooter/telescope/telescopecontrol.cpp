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
#include "indidevapi.h"
#include "indicom.h"
#include "baseclient.h"
#include <QDebug>

class TelescopeControl::Private : public INDI::BaseClient {
public:
  Private(TelescopeControl *q);
  virtual void newBLOB(IBLOB* bp);
  virtual void newDevice(INDI::BaseDevice* dp);
  virtual void newLight(ILightVectorProperty* lvp);
  virtual void newMessage(INDI::BaseDevice* dp, int messageID);
  virtual void newNumber(INumberVectorProperty* nvp);
  virtual void newProperty(INDI::Property* property);
  virtual void newSwitch(ISwitchVectorProperty* svp);
  virtual void newText(ITextVectorProperty* tvp);
  virtual void removeProperty(INDI::Property* property);
  virtual void serverConnected();
  virtual void serverDisconnected(int exit_code);
private:
  TelescopeControl *q;
};

void TelescopeControl::Private::newBLOB(IBLOB* bp)
{

}

void TelescopeControl::Private::newDevice(INDI::BaseDevice* dp)
{

}

void TelescopeControl::Private::newLight(ILightVectorProperty* lvp)
{

}

void TelescopeControl::Private::newMessage(INDI::BaseDevice* dp, int messageID)
{

}

void TelescopeControl::Private::newNumber(INumberVectorProperty* nvp)
{

}

void TelescopeControl::Private::newProperty(INDI::Property* property)
{

}

void TelescopeControl::Private::newSwitch(ISwitchVectorProperty* svp)
{

}

void TelescopeControl::Private::newText(ITextVectorProperty* tvp)
{

}

TelescopeControl::Private::Private(TelescopeControl* q) : q(q)
{

}

void TelescopeControl::Private::removeProperty(INDI::Property* property)
{

}

void TelescopeControl::Private::serverConnected()
{

}

void TelescopeControl::Private::serverDisconnected(int exit_code)
{

}

TelescopeControl::~TelescopeControl()
{
}

TelescopeControl::TelescopeControl(QObject* parent) : d(new Private(this))
{
}

void TelescopeControl::open(QString address, int port)
{
  d->setServer(address.toLocal8Bit().constData(), port);
  qDebug() << "Opening INDI server connection: " << d->connectServer();
}


#include "telescopecontrol.moc"
