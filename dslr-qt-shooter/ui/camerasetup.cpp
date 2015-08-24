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
#include "imagesettingsdialog.h"
#include "commons/shootersettings.h"
#include "ui_camera_setup.h"
#include <QFileDialog>
#include "utils/qt.h"
#include <functional>
#include "Qt/functional.h"

using namespace std;

class CameraSetup::Private {
public:
  Private(ShooterSettings &shooterSettings, CameraSetup *q);
  ShooterSettings &shooterSettings;
  unique_ptr<Ui::CameraSetup> ui;
  QButtonGroup *fileOutput;
    ImagerPtr imager;
  void load();
  void camera_settings(function<void(Imager::Settings::ptr)> callback);
private:
  CameraSetup *q;
};

CameraSetup::Private::Private(ShooterSettings& shooterSettings, CameraSetup* q) : shooterSettings{shooterSettings}, ui{new Ui::CameraSetup()}, q{q}
{
}


void CameraSetup::Private::camera_settings(function<void(Imager::Settings::ptr)> callback)
{
  if(!imager)
    return;
//   ui->shoot->setDisabled(true); TODO
  ui->imageSettings->setDisabled(true);
  qt_async<Imager::Settings::ptr>([=]{ return imager->settings(); }, [=](const Imager::Settings::ptr &settings) {
    callback(settings);
//     ui->shoot->setEnabled(true); TODO
    ui->imageSettings->setEnabled(true);
  });
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
  ui->imageSettings->setEnabled(imager.operator bool());
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
  connect(d->ui->shoot_mode, F_PTR(QComboBox, currentIndexChanged, int), [=](int index){
    d->shooterSettings.shootMode( index == 0 ? ShooterSettings::Single : ShooterSettings::Repeat);
    d->load();
  });
  connect(d->ui->ditherAfterShot, &QCheckBox::toggled, [=](bool checked){
    d->shooterSettings.ditherAfterEachShot(checked);
    d->load();
  });
  connect(d->ui->images_count, F_PTR(QSpinBox, valueChanged, int), [=](int v){
    d->shooterSettings.sequenceLength(v);
    d->load();
  });
  
  connect(d->ui->shoot_interval, &QTimeEdit::timeChanged, [=](QTime t) {
    d->shooterSettings.delayBetweenShots(t);
    d->load();
  });
  d->load();
}


void CameraSetup::shooting(bool isShooting)
{
  d->ui->shoot_mode->setEnabled(!isShooting);
  d->ui->shoot_interval->setEnabled(!isShooting);
  d->ui->images_count->setEnabled(!isShooting);
  d->ui->imageSettings->setEnabled(!isShooting);
}

void CameraSetup::setCamera(const ImagerPtr& imager)
{
  d->ui->imageSettings->disconnect();
  d->imager = imager;
  if(imager) {
    d->camera_settings([=](const Imager::Settings::ptr &settings) {
    auto camera_settings = d->shooterSettings.camera(imager, settings);
    settings->setSerialShootPort(camera_settings->serialPort());
    settings->setImageFormat(camera_settings->imageFormat());
    settings->setISO(camera_settings->iso());
    settings->setShutterSpeed(camera_settings->shutterSpeed());
    settings->setManualExposure(camera_settings->manualExposure());
  });
  
    
    auto reloadSettings = [=] {
    d->camera_settings([=](const Imager::Settings::ptr &settings){
      d->ui->isoLabel->setText(settings->iso().current);
      d->ui->imageFormatLabel->setText(settings->imageFormat().current);
      d->ui->shutterSpeedLabel->setText(settings->shutterSpeed().current);
      d->ui->manualExposureLabel->setText(QTime(0,0,0).addSecs(settings->manualExposure()).toString());
      
      auto camera_settings = d->shooterSettings.camera(imager, settings);
      camera_settings->imageFormat(settings->imageFormat().current);
      camera_settings->serialPort(settings->serialShootPort());
      camera_settings->iso(settings->iso().current);
      camera_settings->shutterSpeed(settings->shutterSpeed().current);
      camera_settings->manualExposure(settings->manualExposure());
    });
  };

  timedLambda(500, reloadSettings, this);
  
  connect(d->ui->imageSettings, &QPushButton::clicked, [=]{
    d->camera_settings([=](const Imager::Settings::ptr &settings){
      auto dialog = new ImageSettingsDialog{ settings , this};
      connect(dialog, &QDialog::accepted, [=]{ d->ui->imageSettings->setEnabled(true); timedLambda(500, reloadSettings, this);});
      dialog->show();
    });
  });
  }
  d->load();
}


#include "camerasetup.moc"
