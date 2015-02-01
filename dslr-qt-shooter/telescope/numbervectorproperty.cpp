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
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

NumberVectorProperty::~NumberVectorProperty()
{
}

NumberVectorProperty::NumberVectorProperty(INumberVectorProperty* property, const std::shared_ptr< INDIClient >& indiClient, QWidget* parent)
  : QGroupBox(parent), VectorProperty(property, indiClient, this)
{
  connect(indiClient.get(), &INDIClient::newNumber, this, [=](INumberVectorProperty *p) { load(p, p->nnp); }, Qt::QueuedConnection);
  load(property, property->nnp);
}

QWidget* NumberVectorProperty::propertyWidget(int index)
{
  INumber sw = _property->np[index];
  qDebug() << "label: " << sw.label << ", name: " << sw.name;
  
  QWidget *widget = new QWidget;
  QHBoxLayout *layout = new QHBoxLayout(widget);
  widget->setLayout(layout);
  layout->addWidget(new QLabel(sw.label));
  QDoubleSpinBox *edit = new QDoubleSpinBox;
  edit->setValue(sw.value);
  edit->setMinimum(sw.min);
  edit->setMaximum(sw.max);
  edit->setSingleStep(sw.step);
  layout->addWidget(edit);
  QPushButton *change = new QPushButton(tr("set"));
  layout->addWidget(change);
  connect(change, &QPushButton::clicked, [=]() {
    _property->np[index].value = edit->value();
    _indiClient->sendNewNumber(_property);
  });
  return widget;
}


#include "numbervectorproperty.moc"
