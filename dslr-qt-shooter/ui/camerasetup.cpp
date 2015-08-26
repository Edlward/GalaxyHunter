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
#include "imaging/imagingsequence.h"

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
  Imager::Settings::ptr imagerSettings;
  void show_settings();
private:
  CameraSetup *q;
};

CameraSetup::Private::Private(ShooterSettings& shooterSettings, CameraSetup* q)
  : shooterSettings{shooterSettings},
  ui{new Ui::CameraSetup()}, q{q}
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
   imagerSettings = settings;
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


void CameraSetup::Private::show_settings()
{
  ui->isoLabel->setText(imagerSettings->iso().current);
  ui->imageFormatLabel->setText(imagerSettings->imageFormat().current);
  ui->shutterSpeedLabel->setText(imagerSettings->shutterSpeed().current);
  ui->manualExposureLabel->setText(QTime(0,0,0).addSecs(imagerSettings->manualExposure()).toString());
  
  auto camera_settings = shooterSettings.camera(imager, imagerSettings);
  camera_settings->imageFormat(imagerSettings->imageFormat().current);
  camera_settings->serialPort(imagerSettings->serialShootPort());
  camera_settings->iso(imagerSettings->iso().current);
  camera_settings->shutterSpeed(imagerSettings->shutterSpeed().current);
  camera_settings->manualExposure(imagerSettings->manualExposure());
}


void CameraSetup::setCamera(const ImagerPtr& imager)
{    
  d->ui->imageSettings->disconnect();
  d->ui->imageSettings->setDisabled(true);
  d->imager = imager;
  if(imager) {
    d->camera_settings([=](const Imager::Settings::ptr &settings) {
    d->imagerSettings = settings;
    d->show_settings();
    d->ui->imageSettings->setEnabled(true);
  });
  
  connect(d->ui->imageSettings, &QPushButton::clicked, [=]{
    auto dialog = new ImageSettingsDialog{ d->imagerSettings, this};
    connect(dialog, &QDialog::accepted, bind(&Private::show_settings, d.get()));
    dialog->show();
  });
  }
  d->load();
}

shared_ptr< ImagingSequence > CameraSetup::imagingSequence() const
{
  int sequenceLength = 1;
  if(d->shooterSettings.shootMode() == ShooterSettings::Repeat) {
    sequenceLength = d->shooterSettings.sequenceLength() == 0 ? std::numeric_limits<int>().max() : d->shooterSettings.sequenceLength();
  }
  ImagingSequence::SequenceSettings sequenceSettings{sequenceLength, d->shooterSettings.delayBetweenShots(), false, d->shooterSettings.saveImage(), d->shooterSettings.saveImageDirectory()};
  return make_shared<ImagingSequence>(d->imager, d->imagerSettings, sequenceSettings);
}


#include "camerasetup.moc"
