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
#include <QFileDialog>
#include <QPushButton>

using namespace std;

class ImageSettingsDialog::Private {
public:
  Private(ImageSettingsDialog *q, Ui::ImageSettingsDialog *ui, Imager::Settings &imagerSettings)
    : q(q), ui(ui), imagerSettings(imagerSettings) {
    }
    ImageSettingsDialog *q;
    unique_ptr<Ui::ImageSettingsDialog> ui;
    Imager::Settings &imagerSettings;
};

ImageSettingsDialog::~ImageSettingsDialog()
{
}

ImageSettingsDialog::ImageSettingsDialog( Imager::Settings& imagerSettings, QWidget* parent )
  : QDialog(parent), dptr(this, new Ui::ImageSettingsDialog, imagerSettings)
{
    d->ui->setupUi(this);
  auto populateCombo = [=] (QComboBox *combo, const Imager::Settings::ComboSetting &setting) {
    combo->setEnabled(setting);
    for(auto value: setting.available) {
      combo->addItem(value);
    }
    combo->setCurrentText(setting.current);
  };
  
  populateCombo(d->ui->imageFormat, d->imagerSettings.imageFormat);
  populateCombo(d->ui->iso, d->imagerSettings.iso);
  populateCombo(d->ui->shutterSpeedPresets, d->imagerSettings.shutterSpeed);
  d->ui->mirror_lock->setChecked(d->imagerSettings.mirrorLock);
  connect(d->ui->mirror_lock, &QCheckBox::toggled, [=](bool b) {d->imagerSettings.mirrorLock = b; });
  connect(d->ui->shutterSpeedPresets, &QComboBox::currentTextChanged, [=](const QString &t) { d->imagerSettings.shutterSpeed.current = t; });
  connect(d->ui->iso, &QComboBox::currentTextChanged, [=](const QString &t) { d->imagerSettings.iso.current = t; });
  connect(d->ui->imageFormat, &QComboBox::currentTextChanged, [=](const QString &t) { d->imagerSettings.imageFormat.current = t; });
  
  auto presetExposure = [=](bool checked) {
    d->ui->shutterSpeedPresets->setEnabled(checked);
    if(!checked)
      d->ui->shutterSpeedPresets->setCurrentText("Bulb");
  };
  auto manualExposure = [=](bool checked) {
    d->ui->shutterSpeedManual->setEnabled(checked);
    d->ui->serialShootPort->setEnabled(checked);
    d->ui->pickSerialShootPort->setEnabled(checked);
    d->ui->shutterSpeedManual->setTime(QTime(0,0,0).addSecs(d->imagerSettings.manualExposureSeconds));
    d->imagerSettings.manualExposure = checked;
  };
  d->ui->serialShootPort->setText(d->imagerSettings.serialShootPort);
  
  connect(d->ui->shutterSpeedManual, &QTimeEdit::timeChanged, [=](const QTime &time) {
    d->imagerSettings.manualExposureSeconds = QTime{0,0,0}.secsTo(d->ui->shutterSpeedManual->time());
  });
  
  connect(d->ui->presetExposure, &QRadioButton::toggled, presetExposure);
  connect(d->ui->manualExposure, &QRadioButton::toggled, manualExposure);
  connect(d->ui->serialShootPort, &QLineEdit::textChanged, [=](const QString &port) { d->imagerSettings.serialShootPort = port; });
  auto bulbMode = d->ui->shutterSpeedPresets->currentText() == "Bulb";
  d->ui->presetExposure->setChecked(!bulbMode); presetExposure(!bulbMode);
  d->ui->manualExposure->setChecked(bulbMode); manualExposure(bulbMode);
  connect(d->ui->pickSerialShootPort, &QPushButton::clicked, [=]{
    d->ui->serialShootPort->setText(QFileDialog::getOpenFileName(this, QString(), "/dev"));
  });
}


void ImageSettingsDialog::accept()
{
  qDebug() << __PRETTY_FUNCTION__ << d->imagerSettings;
  QDialog::accept();
  deleteLater();
}
