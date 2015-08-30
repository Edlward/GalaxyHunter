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

#ifndef SEQUENCESWIDGET_H
#define SEQUENCESWIDGET_H

#include <QWidget>
#include "dptr.h"
#include "imaging/sequence_element.h"
class SequencesWidget : public QWidget
{
    Q_OBJECT
public:
    ~SequencesWidget();
    SequencesWidget(ShooterSettings &shooterSettings, QWidget* parent = 0);
    Sequence sequence() const;
    void setImager(const ImagerPtr &imager);
private:
  D_PTR
};

#endif // SEQUENCESWIDGET_H
