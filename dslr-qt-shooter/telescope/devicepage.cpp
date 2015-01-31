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

#include "devicepage.h"
#include "switchvectorproperty.h"
#include "indiclient.h"
#include <QBoxLayout>
#include <QLabel>
#include <qlayoutitem.h>
#include <QLine>

class DevicePage::Private {
public:
  Private(INDI::BaseDevice *device, const std::shared_ptr<INDIClient> &client, DevicePage *q);
  INDI::BaseDevice *device;
  std::shared_ptr<INDIClient> client;
private:
  DevicePage *q;
};


DevicePage::Private::Private(INDI::BaseDevice* device, const std::shared_ptr< INDIClient >& client, DevicePage* q) : device(device), client(client), q(q)
{

}


DevicePage::~DevicePage()
{
}

DevicePage::DevicePage(INDI::BaseDevice *device, const std::shared_ptr<INDIClient> &indiClient, QWidget* parent) : QWidget(parent), d(new Private{device, indiClient, this})
{
  QBoxLayout *layout;
  setLayout(layout = new QVBoxLayout);
  layout->addWidget(new QLabel(device->getDeviceName(), this) );
  auto hLine = new QFrame(this);
  hLine->setFrameShape(QFrame::HLine);
  hLine->setFrameShadow(QFrame::Sunken);
  hLine->setLineWidth(1);
  layout->addWidget(hLine);
  for(INDI::Property *property: *device->getProperties()) {
    if(property->getType() == INDI_SWITCH) {
      SwitchVectorProperty *switchWidget = new SwitchVectorProperty(property->getSwitch(), indiClient, this);
      layout->addWidget(switchWidget);
    }
  }
  layout->addStretch();
}

#include "devicepage.moc"
