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

#include "textvectorproperty.h"
#include <QLineEdit>
#include <QLabel>
#include <QDebug>
#include <QPushButton>

TextVectorProperty::~TextVectorProperty()
{
}

TextVectorProperty::TextVectorProperty(ITextVectorProperty* property, const std::shared_ptr< INDIClient >& indiClient, QWidget* parent)
  : QGroupBox(parent), VectorProperty(property, indiClient, this)
{
  connect(indiClient.get(), &INDIClient::newText, this, [=](ITextVectorProperty *p) { load(p, p->ntp); }, Qt::QueuedConnection);
  load(property, property->ntp);
}

QWidget* TextVectorProperty::propertyWidget(int index)
{
  IText sw = _property->tp[index];
  qDebug() << "label: " << sw.label << ", name: " << sw.name;
  
  QWidget *widget = new QWidget;
  QHBoxLayout *layout = new QHBoxLayout(widget);
  widget->setLayout(layout);
  layout->addWidget(new QLabel(sw.label));
  QLineEdit *lineEdit = new QLineEdit(sw.text);
  layout->addWidget(lineEdit);
  QPushButton *change = new QPushButton(tr("set"));
  layout->addWidget(change);
  connect(change, &QPushButton::clicked, [=]() {
    qDebug() << lineEdit->text() << "set: ";
    IUSaveText(&_property->tp[index], lineEdit->text().toLocal8Bit().constData());
    _indiClient->sendNewText(_property);
  });
  return widget;
}

#include "textvectorproperty.moc"
