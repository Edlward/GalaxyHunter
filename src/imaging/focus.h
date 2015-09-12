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

#ifndef FOCUS_H
#define FOCUS_H

#include <QObject>
#include <QVector>

class Focus : public QObject
{
    Q_OBJECT
public:
    ~Focus();
    Focus(QObject* parent = 0);
    QVector<double> history() const { return _history; }
    void clear_history() { _history.clear(); }
public slots:
  void analyze(const QImage &image);

signals:
  void focus_rate(double);
private:
  QVector<double> _history;
};

#endif // FOCUS_H
