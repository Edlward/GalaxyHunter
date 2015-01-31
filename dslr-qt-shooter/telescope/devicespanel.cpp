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

#include "devicespanel.h"
#include "telescopecontrol.h"
#include "indiclient.h"
#include "devicepage.h"
#include "ui_devicespanel.h"
#include <QStandardItemModel>
#include "basedevice.h"

class DevicesPanel::Private {
public:
  Private(const std::shared_ptr<INDIClient> &indiClient, DevicesPanel *q);
    void populateDevices();
      std::shared_ptr<INDIClient> indiClient;
      Ui::DevicesPanel* ui;
      QStandardItemModel devices;
      template<typename T> long long l_ptr(T *t) const { return reinterpret_cast<long long>(t); }
      template<typename T> T* ptr_l(long long l) const { return reinterpret_cast<T*>(l); }
      void showDevicePage(INDI::BaseDevice *device);
      QMap<INDI::BaseDevice*, DevicePage*> pages;
private:
  DevicesPanel *q;
};

DevicesPanel::Private::Private(const std::shared_ptr< INDIClient >& indiClient, DevicesPanel* q) : indiClient(indiClient), q(q)
{
}


void DevicesPanel::Private::populateDevices()
{
  devices.clear();
  for(auto device: indiClient->devices()) {
    QStandardItem *deviceItem = new QStandardItem(device->getDeviceName());
    deviceItem->setData(l_ptr(device));
    devices.appendRow(deviceItem);
  }
}


DevicesPanel::~DevicesPanel()
{
}

DevicesPanel::DevicesPanel(const std::shared_ptr<INDIClient> &indiClient, QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f), d(new Private{indiClient, this})
{
    d->ui = new Ui::DevicesPanel;
    d->ui->setupUi(this);
    d->ui->devices->setModel(&d->devices);
    d->populateDevices();
    connect(d->indiClient.get(), &INDIClient::devicesUpdated, [=] { d->populateDevices(); });
    connect(d->ui->devices, &QListView::activated, [=](const QModelIndex &i) {
      QStandardItem *item = d->devices.itemFromIndex(i);
      d->showDevicePage( d->ptr_l<INDI::BaseDevice>( item->data().toLongLong() ) );
    });  
}

void DevicesPanel::Private::showDevicePage(INDI::BaseDevice* device)
{
  if(pages.count(device) == 0) {
    pages[device] = new DevicePage{device, indiClient};
    ui->devicePanel->addWidget(pages[device]);
  }
  ui->devicePanel->setCurrentWidget(pages[device]);
  qDebug() << "On Device: " << device->getDeviceName();
  static std::map<INDI_TYPE, QString> types {
    {INDI_NUMBER, "INDI_NUMBER"}, /*!< INumberVectorProperty. */
    {INDI_SWITCH, "INDI_SWITCH"}, /*!< ISwitchVectorProperty. */
    {INDI_TEXT, "INDI_TEXT"},   /*!< ITextVectorProperty. */
    {INDI_LIGHT, "INDI_LIGHT"},  /*!< ILightVectorProperty. */
    {INDI_BLOB, "INDI_BLOB"},    /*!< IBLOBVectorProperty. */
    {INDI_UNKNOWN, "INDI_UNKNOWN"}
  };
  for(INDI::Property *property: *device->getProperties()) {
    qDebug() << "Property: " << property->getLabel() << ", type: " << types[property->getType()];
  }
}

