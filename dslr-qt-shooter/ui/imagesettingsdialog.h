/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Marco Gulino <email>
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

#ifndef IMAGESETTINGSDIALOG_H
#define IMAGESETTINGSDIALOG_H

#include <QDialog>
#include <memory>
#include "c++/dptr.h"
#include "imaging/imaging_driver.h"
namespace Ui
{
class ImageSettingsDialog;
}

class Imager;
class ImageSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    ~ImageSettingsDialog();
    ImageSettingsDialog(const Imager::Settings::ptr &imagerSettings, QWidget* parent);
public slots:
  void accept();
private:
  D_PTR
};

#endif // IMAGESETTINGSDIALOG_H
