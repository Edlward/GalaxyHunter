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

#ifndef INDIDOUBLECONVERTER_H
#define INDIDOUBLECONVERTER_H

#include <QString>

class INDIDouble
{
public:
  INDIDouble(double value, const QString &format);
  INDIDouble(const QString &text);
  inline bool valid() const { return _valid; }
  inline double value() const { return _value; }
  inline QString text() const { return _text; }
  inline operator bool() const { return valid(); }
  inline operator double() const { return value(); }
  inline operator QString() const { return text(); }
private:
  double _value;
  QString _text;
  bool _valid = false;
};

#endif // INDIDOUBLECONVERTER_H
