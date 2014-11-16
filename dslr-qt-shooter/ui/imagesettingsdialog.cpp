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

#include "imagesettingsdialog.h"
#include "ui_imagesettingsdialog.h"
#include "imaging/imaging_driver.h"
#include <utils/qt.h>
#include <utils/qlambdathread.h>
#include <QThread>

using namespace std;

class ImageSettingsDialog::Private {
public:
  Private(ImageSettingsDialog *q, Ui::ImageSettingsDialog *ui, const shared_ptr<Imager> &imager)
    : q(q), ui(ui), imager(imager), shutterSpeed(imager->shutterSpeed().current), iso(imager->iso().current), imageFormat(imager->imageFormat().current) {}
    ImageSettingsDialog *q;
    unique_ptr<Ui::ImageSettingsDialog> ui;
    shared_ptr<Imager> imager;
    
    // Settings
    QString shutterSpeed;
    QString iso;
    QString imageFormat;
};

ImageSettingsDialog::~ImageSettingsDialog()
{
}

ImageSettingsDialog::ImageSettingsDialog(const shared_ptr<Imager> &imager, QWidget* parent)
  : QDialog(parent), d(new Private{this, new Ui::ImageSettingsDialog, imager})
{
    d->ui->setupUi(this);
    
  auto populateCombo = [=] (QComboBox *combo, const Imager::ComboSetting &setting) {
    combo->setEnabled(setting);
    for(auto value: setting.available) {
      combo->addItem(value);
    }
    combo->setCurrentText(setting.current);
  };
  
  populateCombo(d->ui->imageFormat, d->imager->imageFormat());
  populateCombo(d->ui->iso, d->imager->iso());
  populateCombo(d->ui->shutterSpeedPresets, d->imager->shutterSpeed());
  connect(d->ui->shutterSpeedPresets, &QComboBox::currentTextChanged, [=](const QString &t) { d->shutterSpeed = t; });
  connect(d->ui->iso, &QComboBox::currentTextChanged, [=](const QString &t) { d->iso = t; });
  connect(d->ui->imageFormat, &QComboBox::currentTextChanged, [=](const QString &t) { d->imageFormat = t; });
  
  auto presetExposure = [=](bool checked) {
    d->ui->shutterSpeedPresets->setEnabled(checked);
    if(!checked)
      d->ui->shutterSpeedPresets->setCurrentText("Bulb");
  };
  auto manualExposure = [=](bool checked) {
    d->ui->shutterSpeedManual->setEnabled(checked);
    d->ui->shutterSpeedManualUnit->setEnabled(checked);
    d->ui->shutterSpeedManual->setValue(d->imager->manualExposure() > 60 ? d->imager->manualExposure()/60 : d->imager->manualExposure());
    d->ui->shutterSpeedManualUnit->setCurrentIndex(d->imager->manualExposure() > 60 ? 1 : 0);
  };
  
  connect(d->ui->presetExposure, &QRadioButton::toggled, presetExposure);
  connect(d->ui->manualExposure, &QRadioButton::toggled, manualExposure);
  auto bulbMode = d->ui->shutterSpeedPresets->currentText() == "Bulb";
  d->ui->presetExposure->setChecked(!bulbMode); presetExposure(!bulbMode);
  d->ui->manualExposure->setChecked(bulbMode); manualExposure(bulbMode);
}


void ImageSettingsDialog::accept()
{
  uint64_t unit = pow(60, d->ui->shutterSpeedManualUnit->currentIndex());
  uint64_t manualExposure = d->ui->shutterSpeedManual->value() * unit;
  manualExposure = d->ui->manualExposure->isChecked() ? manualExposure : 0;
  new QLambdaThread{d->imager->thread(), [=]{
    d->imager->setShutterSpeed(d->shutterSpeed);
    d->imager->setImageFormat(d->imageFormat);
    d->imager->setISO(d->iso);
    d->imager->setManualExposure(manualExposure);
  }};
  QDialog::accept();
}
