#include "dslr_shooter_window.h"
#include "imagesettingsdialog.h"
#include "ui_dslr_shooter_window.h"
#include "guider/linguider.h"
#include <QtCore/QTimer>
#include "imaging/imaging_driver.h"
#include <utils/qt.h>
#include <QDebug>
#include <QThread>
#include <QScrollBar>
#include <QFileDialog>
#include <QScreen>
#include <QSettings>
#include <QtConcurrent>
#include <QComboBox>
#include <QMessageBox>

using namespace std;


class DSLR_Shooter_Window::Private {
public:
  Private(DSLR_Shooter_Window *q, Ui::DSLR_Shooter_Window *ui, ImagingDriver *imagingDriver) : q(q), ui(ui), imagingDriver(imagingDriver),
    settings("GuLinux", "DSLR-Shooter") {}
    unique_ptr<Ui::DSLR_Shooter_Window> ui;
    LinGuider *guider;
    struct LogEntry {
       QString message;
       QDateTime when;
    };

    QList<LogEntry> logEntries;
    ImagingDriver *imagingDriver;
    shared_ptr<Imager> imager;
    QSettings settings;
    bool abort_sequence;
    
    void enableOrDisableShootingModeWidgets();
    void camera_settings(function<void(Imager::Settings::ptr)> callback);
    
    void shoot(std::shared_ptr< long int > remaining, function< void() > afterShot, function< void() > afterSequence);
    
private:
  DSLR_Shooter_Window *q;
};


DSLR_Shooter_Window::DSLR_Shooter_Window(QWidget *parent) :
  QMainWindow(parent), d(new Private(this, new Ui::DSLR_Shooter_Window, ImagingDriver::imagingDriver() ))
{
  d->ui->setupUi(this);
  QMenu *setCamera = new QMenu("Available Cameras", this);
  d->ui->actionSet_Camera->setMenu(setCamera);
  connect(d->ui->actionClean_Log_Messages, &QAction::triggered, [=]{ d->ui->logWindow->clear(); });
  connect(d->ui->stopShooting, &QPushButton::clicked, [=]{ d->ui->stopShooting->setDisabled(true); d->abort_sequence = true; });
  connect(d->ui->action_Quit, SIGNAL(triggered(bool)), qApp, SLOT(quit()));
  d->ui->toolBox->setEnabled(false);
  d->ui->toolBox->setCurrentIndex(0);
  d->guider = new LinGuider(this);
  QTimer *updateTimer = new QTimer();
  connect(updateTimer, SIGNAL(timeout()), this, SLOT(update_infos()));
  updateTimer->start(2000);
  QThread *imaging_thread = new QThread(this);
  d->imagingDriver->moveToThread(imaging_thread);
  imaging_thread->start();
  
  resize(QGuiApplication::primaryScreen()->availableSize() * 4 / 5);
  d->ui->imageContainer->setWidgetResizable(true);
  d->ui->camera_splitter->setSizes({height()/10*8, height()/10*2});
  d->ui->log_splitter->setSizes({height()/10*8, height()/10*2});
  d->ui->stopShooting->setHidden(true);
  
  auto set_imager = [=](const shared_ptr<Imager> &imager) {
    d->imager = imager;
    connect(d->imager.get(), SIGNAL(connected()), this, SLOT(camera_connected()), Qt::QueuedConnection);
    connect(d->imager.get(), SIGNAL(disconnected()), this, SLOT(camera_disconnected()), Qt::QueuedConnection);
    connect(d->imager.get(), &Imager::exposure_remaining, this, [=](int seconds){
      statusBar()->showMessage(tr("Exposure remaining: %1").arg(QTime(0,0,0).addSecs(seconds).toString()));
    }, Qt::QueuedConnection);
    d->imager->connect();
  };

  connect(d->imagingDriver, SIGNAL(imager_error(QString)), this, SLOT(got_error(QString)), Qt::QueuedConnection);
  connect(d->imagingDriver, SIGNAL(imager_message(QString)), this, SLOT(got_message(QString)), Qt::QueuedConnection);
  connect(d->imagingDriver, SIGNAL(camera_connected()), this, SLOT(camera_connected()), Qt::QueuedConnection);
  
  connect(d->ui->zoomIn, &QPushButton::clicked, [=] { d->ui->imageContainer->scale(1.2); });
  connect(d->ui->zoomOut, &QPushButton::clicked, [=] { d->ui->imageContainer->scale(0.8); });
  connect(d->ui->zoomActualSize, &QPushButton::clicked, [=] { d->ui->imageContainer->normalSize(); });
  connect(d->ui->zoomFit, &QPushButton::clicked, [=] { d->ui->imageContainer->fitToWindow(); });
  connect(d->ui->shoot, SIGNAL(clicked(bool)), this, SLOT(start_shooting()));
  connect(d->ui->actionScan, SIGNAL(triggered(bool)), d->imagingDriver, SLOT(scan()), Qt::QueuedConnection);
  connect(d->imagingDriver, &ImagingDriver::scan_finished, this, [=]{
    setCamera->clear();
      qDebug() << __PRETTY_FUNCTION__ << ": cameras size: " << d->imagingDriver->imagers().size();
    for(auto camera: d->imagingDriver->imagers()) {
      connect(setCamera->addAction(camera->model()), &QAction::triggered, [=] {
        set_imager(camera);
      });
    }
  }, Qt::QueuedConnection);

  auto outputChanged = [=] (bool save) {
    d->ui->outputDir->setEnabled(save);
    d->ui->outputDirButton->setEnabled(save);
    QMetaObject::invokeMethod(d->imager.get(), "setOutputDirectory", Qt::QueuedConnection, Q_ARG(QString, save ? d->ui->outputDir->text() : QString() ));
  };
  connect(d->ui->outputDiscard, &QAbstractButton::toggled, [=](bool dontsave) { outputChanged(!dontsave); });
  connect(d->ui->outputSave, &QAbstractButton::toggled, outputChanged);
  connect(d->ui->outputDirButton, &QAbstractButton::clicked, [=]{
    d->ui->outputDir->setText(QFileDialog::getExistingDirectory());
    outputChanged(true);
  });
  
  
  connect(d->ui->shoot_interval, &QTimeEdit::timeChanged, [=](const QTime &t) { d->settings.setValue("shoot_interval", t); });
  connect(d->ui->ditherAfterShot, &QCheckBox::toggled, [=](bool t) { d->settings.setValue("dither_after_each_shot", t); });
  connect(d->ui->shoot_mode, SIGNAL(activated(int)), this, SLOT(shootModeChanged(int)));
}

// TODO: why doesn't it work with lambda slot?
void DSLR_Shooter_Window::shootModeChanged(int index)
{
  d->settings.setValue("shoot_mode", d->ui->shoot_mode->currentIndex());
  d->enableOrDisableShootingModeWidgets();
}



DSLR_Shooter_Window::~DSLR_Shooter_Window()
{
}

void DSLR_Shooter_Window::update_infos()
{
  QString message = d->guider->is_connected() ? "Connected" : "Disconnected";
  if(d->guider->is_connected())
    message += " - " + d->guider->version();
  d->ui->guiderStatus->setText(message);
}


void DSLR_Shooter_Window::on_connectLinGuider_clicked()
{
    d->guider->connect();
    update_infos();
}

void DSLR_Shooter_Window::on_dither_clicked()
{
    auto response = d->guider->dither();
    d->logEntries.prepend({response, QDateTime::currentDateTime()});
    update_log();
}

void DSLR_Shooter_Window::update_log()
{
    d->ui->logWindow->clear();
    QStringList log;
    transform(d->logEntries.begin(), d->logEntries.end(), back_inserter(log), [](const Private::LogEntry &e) { return QString("%1 - %2").arg(e.when.toString(Qt::ISODate)).arg(e.message); } );
    d->ui->logWindow->setText(log.join("\n"));
}

void DSLR_Shooter_Window::got_error(const QString& error)
{
  if(error.isEmpty())
    return;
  d->logEntries.prepend({error, QDateTime::currentDateTime()});
  update_log();
}

void DSLR_Shooter_Window::got_message(const QString& message)
{
  if(message.isEmpty())
    return;
  d->logEntries.prepend({message, QDateTime::currentDateTime()});
  update_log();
}

void DSLR_Shooter_Window::Private::enableOrDisableShootingModeWidgets()
{
  auto enable = ui->shoot_mode->currentIndex() == 1;
  ui->shoot_interval->setEnabled(enable);
  ui->ditherAfterShot->setEnabled(enable);
  ui->images_count->setEnabled(enable);
}

void DSLR_Shooter_Window::Private::camera_settings(function<void(Imager::Settings::ptr)> callback)
{
  if(!imager)
    return;
  ui->shoot->setDisabled(true);
  ui->imageSettings->setDisabled(true);
  qt_async<Imager::Settings::ptr>([=]{ return imager->settings(); }, [=](const Imager::Settings::ptr &settings) {
    callback(settings);
    ui->shoot->setEnabled(true);
    ui->imageSettings->setEnabled(true);
  });
}



void DSLR_Shooter_Window::camera_connected()
{
  qDebug() << __PRETTY_FUNCTION__;
  d->ui->camera_infos->clear();
  
  d->ui->shoot_mode->setCurrentIndex(d->settings.value("shoot_mode", 0).toInt());
  d->ui->shoot_interval->setTime(d->settings.value("shoot_interval", QTime(0,0,0)).toTime());
  d->ui->ditherAfterShot->setChecked(d->settings.value("dither_after_each_shot", false).toBool());
  
  d->enableOrDisableShootingModeWidgets();
  
  d->camera_settings([=](const Imager::Settings::ptr &settings) {
    d->settings.beginGroup(QString("camera %1").arg(d->imager->model()));
    settings->setSerialShootPort(d->settings.value("serial_shoot_port", "/dev/ttyUSB0").toString().toStdString());
    settings->setImageFormat(d->settings.value("image_format", settings->imageFormat().current).toString());
    settings->setISO(d->settings.value("iso", settings->iso().current).toString());
    settings->setShutterSpeed(d->settings.value("shutter_speed", settings->shutterSpeed().current).toString());
    settings->setManualExposure(d->settings.value("manual_exposure_secs", settings->manualExposure()).toULongLong());
    d->settings.endGroup();
  });
  
  QString camera_infos = QString("Model: %1\nSummary: %2")
    .arg(d->imager->model())
    .arg(d->imager->summary());
  got_message(QString("Camera connected: %1").arg(d->imager->model()));
  d->ui->camera_infos->setText(camera_infos);
  d->ui->shoot->setEnabled(true);
  d->ui->toolBox->setEnabled(true);

  d->ui->imageSettings->disconnect();
  
  auto reloadSettings = [=] {
    d->camera_settings([=](const Imager::Settings::ptr &settings){
      d->ui->isoLabel->setText(settings->iso().current);
      d->ui->imageFormatLabel->setText(settings->imageFormat().current);
      d->ui->shutterSpeedLabel->setText(settings->shutterSpeed().current);
      d->ui->manualExposureLabel->setText(QTime(0,0,0).addSecs(settings->manualExposure()).toString());
      
      d->settings.beginGroup(QString("camera %1").arg(d->imager->model()));
      d->settings.setValue("serial_shoot_port", QString::fromStdString(settings->serialShootPort()));
      d->settings.setValue("image_format", settings->imageFormat().current);
      d->settings.setValue("iso", settings->iso().current);
      d->settings.setValue("shutter_speed", settings->shutterSpeed().current);
      d->settings.setValue("manual_exposure_secs", settings->manualExposure());
      d->settings.endGroup();
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


void DSLR_Shooter_Window::Private::shoot(std::shared_ptr<long> remaining, std::function< void()> afterShot, std::function< void()> afterSequence)
{
    qDebug() << "Shots remaining: " << *remaining;
    if(*remaining <= 0 || abort_sequence) {
      afterSequence();
      ui->stopShooting->setEnabled(true);
      return;
    }
    qt_async<QImage>([=]{ return imager->shoot();}, [=](const QImage &image) {
      --*remaining;
      ui->imageContainer->setImage(image);
      for(auto widget: vector<QAbstractButton*>{ui->zoomActualSize, ui->zoomFit, ui->zoomIn, ui->zoomOut})
	widget->setEnabled(!image.isNull());
      afterShot();
      long seconds_interval = QTime{0,0,0}.secsTo(ui->shoot_interval->time());
      timedLambda(seconds_interval * 1000, [=]{ shoot(remaining, afterShot, afterSequence); }, q);
    });
}


void DSLR_Shooter_Window::start_shooting()
{
  long total_shots_number = d->ui->shoot_mode->currentIndex() == 0 ? 1 : d->ui->images_count->value();
  shared_ptr<long> remaining_shots = make_shared<long>(total_shots_number);
  d->abort_sequence = false;
  auto setWidgetsEnabled = [=](bool enable) {
    d->ui->shoot_mode->setEnabled(enable);
    d->ui->shoot_interval->setEnabled(enable);
    d->ui->images_count->setEnabled(enable);
    d->ui->imageSettings->setEnabled(enable);
    d->ui->shoot->setEnabled(enable);
  };
  setWidgetsEnabled(false);
  d->ui->stopShooting->setVisible(total_shots_number>1);
  d->shoot(remaining_shots, [=]{
    d->ui->images_count->setValue(*remaining_shots);
    qDebug() << "Checking for dithering...";
    if(d->ui->ditherAfterShot->isChecked() && d->guider->is_connected()) {
      qDebug() << "Dither enabled: dithering";
      d->guider->dither();
    }
  }, [=]{
    d->ui->images_count->setValue(total_shots_number);
    d->ui->stopShooting->setHidden(true);
    setWidgetsEnabled(true);
    statusBar()->clearMessage();
  } );
}

void DSLR_Shooter_Window::camera_disconnected()
{
  d->ui->shoot->setDisabled(true);
  disconnect(d->ui->imageContainer, SLOT(setImage(const QImage &)));
}

