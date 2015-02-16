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

#include "indiclient.h"
#include <ui/logmessage.h>

#include "indidevapi.h"
#include "indicom.h"
#include "baseclient.h"
#include <basedevice.h>
#include <inditelescope.h>
#include <QDebug>

class INDIClient::Private : public INDI::BaseClient {
public:
  Private(INDIClient *q);
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
  INDIClient *q;
};

void INDIClient::Private::newBLOB(IBLOB* bp)
{
}

void INDIClient::Private::newDevice(INDI::BaseDevice* dp)
{
  q->devicesUpdated();
}

void INDIClient::Private::newLight(ILightVectorProperty* lvp)
{
  q->newLight(lvp);
}

void INDIClient::Private::newMessage(INDI::BaseDevice* dp, int messageID)
{
  QString message = QString::fromStdString(dp->messageQueue(messageID));
  QRegExp re("[\\d]{4}-[\\d]{2}-[\\d]{2}T[\\d]{2}:[\\d]{2}:[\\d]{2}: (.*)");
  if(re.indexIn(message) != -1) {
    message = re.cap(1);
  }
  q->message(LogMessage::info(QString("indi-%1").arg(dp->getDeviceName()), message));
}

void INDIClient::Private::newNumber(INumberVectorProperty* nvp)
{
  q->newNumber(nvp);
}

void INDIClient::Private::newProperty(INDI::Property* property)
{
  q->propertyAdded(property);
}

void INDIClient::Private::newSwitch(ISwitchVectorProperty* svp)
{
  q->newSwitch(svp);

}

void INDIClient::Private::newText(ITextVectorProperty* tvp)
{
  q->newText(tvp);
}

void INDIClient::Private::removeProperty(INDI::Property* property)
{
  q->propertyRemoved(property);
}

void INDIClient::Private::serverConnected()
{
  qDebug() << __PRETTY_FUNCTION__;
}

void INDIClient::Private::serverDisconnected(int exit_code)
{
  qDebug() << __PRETTY_FUNCTION__ << ": exit code: " <<exit_code;
}


INDIClient::Private::Private(INDIClient* q) : q(q)
{
  qDebug() << __PRETTY_FUNCTION__;
}

INDIClient::~INDIClient()
{
}

INDIClient::INDIClient(QObject* parent) : QObject(parent), d(new Private{this})
{
}

void INDIClient::open(const QString& address, int port)
{
  d->setServer(address.toLocal8Bit().constData(), port);
  qDebug() << "Opening INDI server connection: " << d->connectServer();
}

vector< INDI::BaseDevice *> INDIClient::devices() const
{
  return d->getDevices();
}

void INDIClient::sendNewSwitch(ISwitchVectorProperty* s)
{
  d->sendNewSwitch(s);
}

void INDIClient::sendNewText(ITextVectorProperty* _property)
{
  d->sendNewText(_property);
}

void INDIClient::sendNewNumber(INumberVectorProperty* _property)
{
  d->sendNewNumber(_property);
}


std::vector< INDI::BaseDevice* > INDIClient::telescopes() const
{
  std::vector< INDI::BaseDevice* > _telescopes;
  std::copy_if(begin(d->getDevices()), end(d->getDevices()), back_inserter(_telescopes), [](INDI::BaseDevice* device){
    return 
      device->getProperty("EQUATORIAL_EOD_COORD", INDI_NUMBER) &&
      device->getProperty("TELESCOPE_MOTION_NS", INDI_SWITCH) &&
      device->getProperty("TELESCOPE_MOTION_WE", INDI_SWITCH)
      ;
  });
  return _telescopes;
}


#include "indiclient.moc"
