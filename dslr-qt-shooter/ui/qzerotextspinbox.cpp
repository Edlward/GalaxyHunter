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

#include "qzerotextspinbox.h"

QZeroTextSpinBox::~QZeroTextSpinBox()
{
}

QZeroTextSpinBox::QZeroTextSpinBox(QWidget* parent) : QSpinBox(parent)
{
  validator = new QIntValidator(this);
}

QValidator::State QZeroTextSpinBox::validate(QString& input, int& pos) const
{
  return validator->validate(input, pos);
}

int QZeroTextSpinBox::valueFromText(const QString& text) const
{
  return text.toInt();
}

QString QZeroTextSpinBox::textFromValue(int val) const
{
  return QString::number(val);
}

void QZeroTextSpinBox::setZeroText(const QString& zeroText)
{
  _zeroText = zeroText;
}

QString QZeroTextSpinBox::zeroText() const
{
  return _zeroText;
}


#include "qzerotextspinbox.moc"
