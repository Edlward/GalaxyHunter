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

#ifndef VECTORPROPERTY_H
#define VECTORPROPERTY_H

#include <memory>
#include <QGroupBox>
#include <indiapi.h>
#include <indiproperty.h>
#include <QBoxLayout>
#include <list>
#include "indiclient.h"
#include "ledindicator.h"
#include <QLayoutItem>

class INDIClient;

template<typename T, typename Widget, typename Layout = QHBoxLayout>
class VectorProperty
{
public:
    VectorProperty(T *property,
		   const std::shared_ptr<INDIClient> &indiClient,
		   void (INDIClient::* update_status_f)(T*),
		   QGroupBox *groupBox
		  ) : _property(property), _indiClient(indiClient) {
      groupBox->setTitle(property->label);
      QHBoxLayout *mainLayout = new QHBoxLayout;
      ledIndicator = new LedIndicator(color_for(_property->s));
      groupBox->setLayout(mainLayout);
      mainLayout->addWidget(ledIndicator);
      ledIndicator->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
      mainLayout->addLayout(_layout = new Layout, 1);
      QObject::connect(indiClient.get(), update_status_f, groupBox, [=](T *p) { updateStatus(p->s); load(p); }, Qt::QueuedConnection);
    }
    ~VectorProperty() {}
private:
  LedIndicator *ledIndicator;
  LedIndicator::Color color_for(IPState state) {
  static std::map<IPState, LedIndicator::Color> states = {
	{IPS_ALERT, LedIndicator::Yellow},
	{IPS_BUSY, LedIndicator::Red},
	{IPS_IDLE, LedIndicator::Blue},
	{IPS_OK, LedIndicator::Green},
    };
    return states[state];
  }
  Layout *_layout;
  QSpacerItem *_layout_spacer = 0;
protected:
  T *_property;
  std::shared_ptr<INDIClient> _indiClient;
  std::list<Widget *> _widgets;
  void load(T *property) {
    if(property != _property) return;
    for(auto widget: _widgets) delete widget;
    _widgets.clear();
    for(int i=0; i<property_size(property); i++) {
      auto widget = propertyWidget(i);
      _widgets.push_back(widget);
      _layout->insertWidget(i, widget);
    }
    if(!_layout_spacer)
      _layout->addSpacerItem(_layout_spacer = new QSpacerItem{0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum});
  }
  void updateStatus(IPState state) { ledIndicator->setColor(color_for(state)); }
  virtual int property_size(T *property) const = 0;
  virtual Widget *propertyWidget(int index) = 0;
};

#endif // VECTORPROPERTY_H
