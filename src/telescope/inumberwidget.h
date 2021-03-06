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

#ifndef INUMBERWIDGET_H
#define INUMBERWIDGET_H

#include <QWidget>
#include <functional>

class QPushButton;

class NumberEditor;
class INumberWidget : public QWidget
{
    Q_OBJECT

public:
    ~INumberWidget();
    INumberWidget(const QString &label, const QString &format, QWidget* parent = 0);
  void setOnValueChanged(std::function<void(double)> onValueChanged);
public slots:
  void setValue(double value);
  void setRange(double min, double max);
  void setEnabled(bool enable);

private:
  NumberEditor *numberEditor;
  std::function<void(double)> onValueChanged = [=](double){};
  QPushButton *setButton;
};

#endif // INUMBERWIDGET_H
