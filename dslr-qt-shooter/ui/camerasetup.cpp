/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#include "camerasetup.h"
#include "commons/shootersettings.h"
#include "ui_camera_setup.h"
#include <QFileDialog>

using namespace std;

class CameraSetup::Private {
public:
  Private(ShooterSettings &shooterSettings, CameraSetup *q);
  ShooterSettings &shooterSettings;
  unique_ptr<Ui::CameraSetup> ui;
  QButtonGroup *fileOutput;
  void load();
private:
  CameraSetup *q;
};

CameraSetup::Private::Private(ShooterSettings& shooterSettings, CameraSetup* q) : shooterSettings{shooterSettings}, ui{new Ui::CameraSetup()}, q{q}
{
}

void CameraSetup::Private::load()
{
  auto shootMode = shooterSettings.shootMode();
  ui->shoot_mode->setCurrentIndex(shootMode == ShooterSettings::Single ? 0 : 1);
  ui->repeated_shots_settings->setVisible(shootMode == ShooterSettings::Repeat);
  ui->ditherAfterShot->setChecked(shooterSettings.ditherAfterEachShot());
  ui->images_count->setValue(shooterSettings.sequenceLength());
  ui->shoot_interval->setTime(shooterSettings.delayBetweenShots());
  auto saveImages = shooterSettings.saveImage();
  ui->outputSave->setChecked(saveImages);
  ui->groupBox->setVisible(saveImages ? ui->outputSave : ui->outputSave);
  ui->outputDir->setVisible(saveImages);
  ui->outputDir->setText(shooterSettings.saveImageDirectory());
}


CameraSetup::~CameraSetup()
{
}

CameraSetup::CameraSetup(ShooterSettings& shooterSettings, QWidget* parent) : QWidget(parent), dptr(shooterSettings, this)
{
  d->ui->setupUi(this);
  d->fileOutput = new QButtonGroup(this);
  d->fileOutput->addButton(d->ui->outputDiscard);
  d->fileOutput->addButton(d->ui->outputSave);
  d->fileOutput->setExclusive(true);
  auto pickDirectory = d->ui->outputDir->addAction(QIcon::fromTheme("folder"), QLineEdit::TrailingPosition);
  connect(pickDirectory, &QAction::triggered, [=]{
    auto dir = QFileDialog::getExistingDirectory(this, tr("Save Images Directory"), d->shooterSettings.saveImageDirectory());
    if(dir.isEmpty())
      return;
    d->shooterSettings.saveImageDirectory(dir);
    d->load();
  });
  connect(d->ui->outputSave, &QAbstractButton::toggled, [=](bool save) {
    d->shooterSettings.saveImage(save);
    d->load();
  });
  d->load();
}

#include "camerasetup.moc"
