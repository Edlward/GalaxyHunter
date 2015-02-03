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

#include "lightvectorproperty.h"
#include "ledindicator.h"
#include <QLabel>
#include <QDebug>

LightVectorProperty::~LightVectorProperty()
{
}

LightVectorProperty::LightVectorProperty(ILightVectorProperty* property, const std::shared_ptr< INDIClient >& indiClient, QWidget* parent)
  : QGroupBox(parent), VectorProperty(property, indiClient, &INDIClient::newLight, this)
{
    load(property);
}

int LightVectorProperty::property_size(ILightVectorProperty* property) const
{
  return property->nlp;
}


LedIndicator* LightVectorProperty::propertyWidget(int index)
{
  ILight sw = _property->lp[index];
  static std::map<IPState, LedIndicator::Color> colors {
    { IPS_BUSY, LedIndicator::Red },
    { IPS_OK, LedIndicator::Green },
    { IPS_IDLE, LedIndicator::Blue },
    { IPS_ALERT, LedIndicator::Yellow },
  };
  return new LedIndicator{colors[sw.s], sw.label};
}


#include "lightvectorproperty.moc"
