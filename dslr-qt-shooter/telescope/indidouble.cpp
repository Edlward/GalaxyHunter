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

#include "indidouble.h"
#include <boost/format.hpp>
#include <iostream>
#include <sstream>
#include <libindi/indicom.h>

INDIDouble::INDIDouble(const QString& text, const QString& format) : _text(text), _value(std::numeric_limits<double>::min())
{
  std::stringstream s(text.toStdString());
  s >> _value;
  _valid = true;
//   if(std::numeric_limits<double>::min() == _value) {
//     _value = 0;
//     _valid = false;
//   }
}

INDIDouble::INDIDouble(double value, const QString& format) : _value(value)
{
  char s[256];
  int size = numberFormat(s, format.toLocal8Bit().constData(), _value);
  if(size <= 0)
    return;
  _valid = true;
  _text = QString(s).trimmed();
}
