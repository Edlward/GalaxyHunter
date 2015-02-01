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

class INDIClient;

template<typename T, typename Widget>
class VectorProperty
{
public:
    VectorProperty(T *property, const std::shared_ptr<INDIClient> &indiClient, QGroupBox *groupBox) : _property(property), _indiClient(indiClient) {
      groupBox->setTitle(property->label);
      groupBox->setLayout(_layout = new QHBoxLayout);

    }
    ~VectorProperty() {}
protected:
  T *_property;
  std::shared_ptr<INDIClient> _indiClient;
  QHBoxLayout *_layout;
  virtual Widget *propertyWidget(int index) = 0;
};

#endif // VECTORPROPERTY_H
