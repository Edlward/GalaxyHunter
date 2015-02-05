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

#include "numbervectorproperty.h"
#include "inumberwidget.h"
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

NumberVectorProperty::~NumberVectorProperty()
{
}

NumberVectorProperty::NumberVectorProperty(INumberVectorProperty* property, const std::shared_ptr< INDIClient >& indiClient, QWidget* parent)
  : QGroupBox(parent), VectorProperty(property, indiClient, &INDIClient::newNumber, this)
{
  load(property);
}

#include <iostream>
QWidget* NumberVectorProperty::propertyWidget(int index)
{
  INumber sw = _property->np[index];
  INumberWidget *w = new INumberWidget(sw.label, sw.format);
  w->setEnabled(_property->p != IP_RO);
  w->setRange(sw.min, sw.max);
  w->setValue(sw.value);
  connect(w, &INumberWidget::valueChanged, [=](double v){
    _property->np[index].value = v;
    _indiClient->sendNewNumber(_property);
  });
  return w;
}

int NumberVectorProperty::property_size(INumberVectorProperty* property) const
{
  return property->nnp;
}



#include "numbervectorproperty.moc"
