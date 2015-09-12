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
  void camera_settings(function<void(Imager::Settings)> callback);
  Imager::Settings imagerSettings;
  ImagingSequence::SequenceSettings sequenceSettings;
  void update_settings_ui();
private:
  CameraSetup *q;
};

CameraSetup::Private::Private(ShooterSettings& shooterSettings, CameraSetup* q)
  : shooterSettings{shooterSettings},
  ui{new Ui::CameraSetup()}, q{q}
{
}


void CameraSetup::Private::camera_settings(function<void(Imager::Settings)> callback)
{
  if(!imager)
    return;
//   ui->shoot->setDisabled(true); TODO
  ui->imageSettings->setDisabled(true);
  qt_async<Imager::Settings>([=]{ return imager->settings(); }, [=](const Imager::Settings &settings) {
    callback(settings);
//     ui->shoot->setEnabled(true); TODO
    ui->imageSettings->setEnabled(true);
  });
}

void CameraSetup::Private::load()
{
  ui->shoot_mode->setCurrentIndex(static_cast<int>(sequenceSettings.mode));
  ui->repeated_shots_settings->setVisible(sequenceSettings.mode == ShooterSettings::ShooterSettings::Sequence || sequenceSettings.mode == ShooterSettings::ShooterSettings::Continuous );
  ui->images_count->setEnabled(sequenceSettings.mode == ShooterSettings::ShooterSettings::Sequence);
  ui->ditherAfterShot->setChecked(sequenceSettings.ditherAfterShots);
  ui->images_count->setValue(sequenceSettings.shots);
  ui->shoot_interval->setTime(sequenceSettings.delayBetweenShots);
  auto saveImages = sequenceSettings.saveToDisk;
  ui->outputSave->setChecked(saveImages);
  ui->groupBox->setVisible(saveImages ? ui->outputSave : ui->outputSave);
  ui->outputDir->setVisible(saveImages);
  ui->outputDir->setText(sequenceSettings.saveDirectory);
  ui->imageSettings->setEnabled(imager.operator bool());
  
  shooterSettings.shootMode(sequenceSettings.mode);
  shooterSettings.ditherAfterEachShot(sequenceSettings.ditherAfterShots);
  shooterSettings.sequenceLength(sequenceSettings.shots);
  shooterSettings.delayBetweenShots(sequenceSettings.delayBetweenShots);
  shooterSettings.saveImage(sequenceSettings.saveToDisk);
  shooterSettings.saveImageDirectory(sequenceSettings.saveDirectory);
}


CameraSetup::~CameraSetup()
{
}

CameraSetup::CameraSetup(ShooterSettings& shooterSettings, QWidget* parent) : QWidget(parent), dptr(shooterSettings, this)
{
  d->ui->setupUi(this);
  d->sequenceSettings = { shooterSettings.shootMode(), shooterSettings.sequenceLength(), shooterSettings.delayBetweenShots(), false, 
    shooterSettings.saveImage(), shooterSettings.saveImageDirectory(), shooterSettings.ditherAfterEachShot() };
  d->fileOutput = new QButtonGroup(this);
  d->fileOutput->addButton(d->ui->outputDiscard);
  d->fileOutput->addButton(d->ui->outputSave);
  d->fileOutput->setExclusive(true);
  auto pickDirectory = d->ui->outputDir->addAction(QIcon::fromTheme("folder"), QLineEdit::TrailingPosition);
  connect(pickDirectory, &QAction::triggered, [=]{
    auto dir = QFileDialog::getExistingDirectory(this, tr("Save Images Directory"), d->shooterSettings.saveImageDirectory());
    if(dir.isEmpty())
      return;
    d->sequenceSettings.saveDirectory = dir;
    d->load();
  });
  connect(d->ui->outputSave, &QAbstractButton::toggled, [=](bool save) {
    d->sequenceSettings.saveToDisk = save;
    d->load();
  });
  connect(d->ui->shoot_mode, F_PTR(QComboBox, currentIndexChanged, int), [=](int index){
    d->sequenceSettings.mode = static_cast<ShooterSettings::ShootMode>(index);
    d->load();
  });
  connect(d->ui->ditherAfterShot, &QCheckBox::toggled, [=](bool checked){
    d->sequenceSettings.ditherAfterShots = checked;
    d->load();
  });
  connect(d->ui->images_count, F_PTR(QSpinBox, valueChanged, int), [=](int v){
    d->sequenceSettings.shots = v;
    d->load();
  });
  
  connect(d->ui->shoot_interval, &QTimeEdit::timeChanged, [=](QTime t) {
    d->sequenceSettings.delayBetweenShots = t;
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


void CameraSetup::Private::update_settings_ui()
{
  ui->isoLabel->setText(imagerSettings.iso.current);
  ui->imageFormatLabel->setText(imagerSettings.imageFormat.current);
  ui->shutterSpeedLabel->setText(imagerSettings.shutterSpeed.current);
  ui->manualExposureLabel->setText(QTime(0,0,0).addSecs(imagerSettings.manualExposureSeconds).toString());
  ui->manualExposureLabel->setVisible(imagerSettings.manualExposure);
}


void CameraSetup::setCamera(const ImagerPtr& imager)
{    
  d->ui->imageSettings->disconnect();
  d->ui->imageSettings->setDisabled(true);
  d->imager = imager;
  if(imager) {
    d->camera_settings([=](const Imager::Settings &settings) {
      d->imagerSettings = *d->shooterSettings.camera(imager, settings);
      d->update_settings_ui();
      d->ui->imageSettings->setEnabled(true);
  });
  
  connect(d->ui->imageSettings, &QPushButton::clicked, [=]{
    auto dialog = new ImageSettingsDialog{ d->imagerSettings, this};
    connect(dialog, &QDialog::accepted, [=]{
      d->shooterSettings.camera(imager)->save(d->imagerSettings);
      d->update_settings_ui();
    });
    dialog->show();
  });
  }
  d->load();
}

shared_ptr< ImagingSequence > CameraSetup::imagingSequence() const
{
  return make_shared<ImagingSequence>(d->imager, d->imagerSettings, d->sequenceSettings);
}


#include "camerasetup.moc"
