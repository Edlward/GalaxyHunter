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
#include <QMap>


class TabPage : public QWidget {
  Q_OBJECT
public:
  TabPage(QWidget *parent = 0);
  void addPropertyWidget(QWidget *widget);
private:
  QVBoxLayout *layout;
  QSpacerItem *spacer;
};

TabPage::TabPage(QWidget* parent): QWidget(parent)
{
  setLayout(layout = new QVBoxLayout);
  layout->addSpacerItem(spacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
}
void TabPage::addPropertyWidget(QWidget* widget)
{
  layout->insertWidget(layout->count()-1, widget);
}


class DevicePage::Private {
public:
  Private(INDI::BaseDevice *device, const std::shared_ptr<INDIClient> &client, DevicePage *q);
  INDI::BaseDevice *device;
  std::shared_ptr<INDIClient> client;
  QMap<INDI::Property*, QWidget*> properties;
  TabPage *page(const QString &name);
  QMap<QString, TabPage*> pages;
private:
  DevicePage *q;
};


DevicePage::Private::Private(INDI::BaseDevice* device, const std::shared_ptr< INDIClient >& client, DevicePage* q) : device(device), client(client), q(q)
{

}


DevicePage::~DevicePage()
{
}

DevicePage::DevicePage(INDI::BaseDevice *device, const std::shared_ptr<INDIClient> &indiClient, QWidget* parent) : QTabWidget(parent), d(new Private{device, indiClient, this})
{
//   QBoxLayout *layout;
//   setLayout(layout = new QVBoxLayout);
//   layout->addWidget(new QLabel(device->getDeviceName(), this) );
//   auto hLine = new QFrame(this);
//   hLine->setFrameShape(QFrame::HLine);
//   hLine->setFrameShadow(QFrame::Sunken);
//   hLine->setLineWidth(1);
//   layout->addWidget(hLine);
  
  auto addProperty = [=](INDI::Property *property) {
    if(d->properties.count(property))
      return;
    if(property->getType() == INDI_SWITCH) {
      SwitchVectorProperty *switchWidget = new SwitchVectorProperty(property->getSwitch(), indiClient, this);
      d->properties[property] = switchWidget;
      d->page(property->getGroupName())->addPropertyWidget(switchWidget);
    }
  };
  
  connect(indiClient.get(), &INDIClient::propertyAdded, this, addProperty, Qt::AutoConnection);
  connect(indiClient.get(), &INDIClient::propertyRemoved, this, [=](INDI::Property *p) {
    delete d->properties.value(p, nullptr);
    d->properties.remove(p);
  }, Qt::AutoConnection);
  
  for(INDI::Property *property: *device->getProperties()) {
    addProperty(property);
  }
//   layout->addStretch();
}

TabPage* DevicePage::Private::page(const QString& name)
{
  if(!pages.count(name)) {
    pages[name] = new TabPage;
    q->addTab(pages[name], name);
  }
  return pages[name];
}


#include "devicepage.moc"
