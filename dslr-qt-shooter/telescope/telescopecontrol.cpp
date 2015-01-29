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
#include <basedevice.h>
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
  qDebug() << __PRETTY_FUNCTION__ << ": blob label: " << bp->label << ", name: " << bp->bvp->name;
}

void TelescopeControl::Private::newDevice(INDI::BaseDevice* dp)
{
  qDebug() << __PRETTY_FUNCTION__ << ": device driver: " << dp->getDriverName() << ", name: " << dp->getDeviceName();
}

void TelescopeControl::Private::newLight(ILightVectorProperty* lvp)
{
  qDebug() << __PRETTY_FUNCTION__ << ": label: " << lvp->label << ", name: " << lvp->name;
}

void TelescopeControl::Private::newMessage(INDI::BaseDevice* dp, int messageID)
{
  qDebug() << __PRETTY_FUNCTION__ << ": device=" << dp->getDeviceName() << ", messageID: " << messageID;

}

void TelescopeControl::Private::newNumber(INumberVectorProperty* nvp)
{
  qDebug() << __PRETTY_FUNCTION__;

}

void TelescopeControl::Private::newProperty(INDI::Property* property)
{
  qDebug() << __PRETTY_FUNCTION__ << ": label=" << property->getLabel() << ", name=" << property->getLabel() << ", type=" << property->getType();

}

void TelescopeControl::Private::newSwitch(ISwitchVectorProperty* svp)
{
  qDebug() << __PRETTY_FUNCTION__ << ": label=" << svp->label;

}

void TelescopeControl::Private::newText(ITextVectorProperty* tvp)
{
  qDebug() << __PRETTY_FUNCTION__ << ": label=" << tvp->label;

}

void TelescopeControl::Private::removeProperty(INDI::Property* property)
{
  qDebug() << __PRETTY_FUNCTION__ << ": label=" << property->getName();
}

void TelescopeControl::Private::serverConnected()
{
  qDebug() << __PRETTY_FUNCTION__;
}

void TelescopeControl::Private::serverDisconnected(int exit_code)
{
  qDebug() << __PRETTY_FUNCTION__ << ": exit code: " <<exit_code;
}


TelescopeControl::Private::Private(TelescopeControl* q) : q(q)
{
  qDebug() << __PRETTY_FUNCTION__;

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
