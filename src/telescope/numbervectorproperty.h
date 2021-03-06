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

#ifndef NUMBERVECTORPROPERTY_H
#define NUMBERVECTORPROPERTY_H

#include <QGroupBox>
#include "vectorproperty.h"

class NumberVectorProperty : public QGroupBox, public VectorProperty<INumberVectorProperty, QWidget, QVBoxLayout>
{
    Q_OBJECT

public:
    ~NumberVectorProperty();
    NumberVectorProperty(INumberVectorProperty* property, const std::shared_ptr<INDIClient>& indiClient, QWidget* parent = 0);
protected:
    virtual QWidget* propertyWidget(int index);
    virtual int property_size(INumberVectorProperty* property) const;
private:
};

#endif // NUMBERVECTORPROPERTY_H
