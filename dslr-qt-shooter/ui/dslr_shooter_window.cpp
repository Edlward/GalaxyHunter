#include "dslr_shooter_window.h"
#include "imagesettingsdialog.h"
#include "messageswindow.h"
#include "logmessage.h"
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
#include <QStandardItemModel>
#include <QSettings>
#include <QString>
#include <QtConcurrent>
#include <QComboBox>
#include <QMessageBox>
#include <QLCDNumber>
#include <QSystemTrayIcon>
#include <QInputDialog>
#include "imaging/focus.h"
#include "qwt-src/qwt_plot_curve.h"
#include <qwt-src/qwt_plot_histogram.h>
#include <qwt-src/qwt_symbol.h>
#include "telescope/telescopecontrol.h"

using namespace std;


class DSLR_Shooter_Window::Private {
public:
  Private(DSLR_Shooter_Window *q, Ui::DSLR_Shooter_Window *ui, ImagingDriver *imagingDriver);
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
    Focus *focus;
    QwtPlotCurve *focus_curve;
    TelescopeControl *telescopeControl;
    QStandardItemModel logs;
    QSystemTrayIcon trayIcon;
private:
  DSLR_Shooter_Window *q;
};

DSLR_Shooter_Window::Private::Private(DSLR_Shooter_Window* q, Ui::DSLR_Shooter_Window* ui, ImagingDriver* imagingDriver)
 : q(q), ui(ui), imagingDriver(imagingDriver), settings("GuLinux", "DSLR-Shooter"), trayIcon{QIcon::fromTheme("dslr-qt-shooter")}
{

}


DSLR_Shooter_Window::DSLR_Shooter_Window(QWidget *parent) :
  QMainWindow(parent), d(new Private(this, new Ui::DSLR_Shooter_Window, ImagingDriver::imagingDriver() ))
{
  d->trayIcon.show();
  d->logs.setHorizontalHeaderLabels({tr("Time"), tr("Type"), tr("Source"), tr("Message")});
  d->ui->setupUi(this);
  tabifyDockWidget(d->ui->camera_information_dock, d->ui->camera_setup_dock);
  tabifyDockWidget(d->ui->camera_information_dock, d->ui->guider_dock);
  tabifyDockWidget(d->ui->camera_information_dock, d->ui->focus_dock);
  d->ui->camera_information_dock->raise();
  restoreState(d->settings.value("windows_settings").toByteArray());
  d->telescopeControl = new TelescopeControl(this);
  QMenu *setCamera = new QMenu("Available Cameras", this);
  d->ui->actionSet_Camera->setMenu(setCamera);
  connect(d->telescopeControl, SIGNAL(message(LogMessage)), this, SLOT(got_message(LogMessage)));
  connect(d->ui->stopShooting, &QPushButton::clicked, [=]{ d->ui->stopShooting->setDisabled(true); d->abort_sequence = true; });
  connect(d->ui->action_Quit, SIGNAL(triggered(bool)), qApp, SLOT(quit()));
  connect(d->ui->action_LogMessages, &QAction::triggered, [=]{ (new MessagesWindow{&d->logs})->show(); });
  d->guider = new LinGuider(this);
  QTimer *updateTimer = new QTimer();
  connect(updateTimer, SIGNAL(timeout()), this, SLOT(update_infos()));
  updateTimer->start(2000);
  QThread *imaging_thread = new QThread(this);
  d->imagingDriver->moveToThread(imaging_thread);
  imaging_thread->start();
  
  resize(QGuiApplication::primaryScreen()->availableSize() * 4 / 5);
  d->ui->imageContainer->setWidgetResizable(true);
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

  connect(d->imagingDriver, SIGNAL(imager_message(LogMessage)), this, SLOT(got_message(LogMessage)), Qt::QueuedConnection);
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

  connect(d->ui->focusing_select_roi, &QPushButton::clicked,[=]{
    d->ui->imageContainer->startSelectionMode();
    d->ui->focusing_clear_roi->setEnabled(true);
  });
  connect(d->ui->focusing_clear_roi, &QPushButton::clicked,[=]{
    d->ui->imageContainer->clearROI();
    d->ui->focusing_clear_roi->setEnabled(false);
  });
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
  connect(d->ui->actionConnectTelescope, &QAction::triggered, [=] {
    QString server = QInputDialog::getText(this, tr("Telescope"), tr("Enter telescope address (example: localhost:7624)"), QLineEdit::Normal, "localhost:7624");
    if(server != "") {
      QString address = server.split(":").first();
      int port = server.split(":").last().toInt();
      d->telescopeControl->open(address, port);
    }
  });
  connect(d->ui->action_Devices_Control_Panel, SIGNAL(triggered(bool)), d->telescopeControl, SLOT(showControlPanel()));
  connect(d->ui->actionRemote_Control, SIGNAL(triggered(bool)), d->telescopeControl, SLOT(showTelescopeRemoteControl()));
  QTimer *autoScan = new QTimer(this);
  autoScan->setSingleShot(true);
  connect(autoScan, SIGNAL(timeout()), d->imagingDriver, SLOT(scan()), Qt::QueuedConnection);
  
  d->focus = new Focus;
  QThread *focusThread = new QThread;
  d->focus->moveToThread(focusThread);
  focusThread->start();
  d->focus_curve= new QwtPlotCurve;
  d->focus_curve->attach(d->ui->focusing_graph);
  d->focus_curve->setStyle(QwtPlotCurve::Lines);
  //d->focus_curve->setCurveAttribute(QwtPlotCurve::Fitted);
  auto symbol = new QwtSymbol(QwtSymbol::Diamond);
  symbol->setSize(8, 8);
  symbol->setBrush(QBrush{Qt::red});
  d->focus_curve->setSymbol(symbol);
  d->focus_curve->setBrush(QBrush{Qt::blue});
  d->ui->focusing_graph->setAutoReplot(true);
  d->ui->focusing_graph->setAxisAutoScale(true);
  connect(d->focus, SIGNAL(focus_rate(double)), this, SLOT(focus_received(double)), Qt::QueuedConnection);
  connect(d->ui->enable_focus_analysis, &QCheckBox::toggled, [=](bool checked) {
    d->ui->focusing_select_roi->setEnabled(checked);
    if(!checked)
      d->ui->imageContainer->clearROI();
  });
  //d->ui->focusing_graph->resize(d->ui->toolBox->width(), d->ui->toolBox->width()*1.5);
  autoScan->start(1000);
}

// TODO: why doesn't it work with lambda slot?
void DSLR_Shooter_Window::shootModeChanged(int index)
{
  d->settings.setValue("shoot_mode", d->ui->shoot_mode->currentIndex());
  d->enableOrDisableShootingModeWidgets();
}

void DSLR_Shooter_Window::focus_received(double value)
{
    qDebug() << "got focus HFD: " << value;
    d->ui->focus_analysis_value->display(value);
    auto history = d->focus->history();
    
    QStringList latest_history;
//     bool first = true;
//     std::transform(begin(d->focus->history()), end(d->focus->history()), back_inserter(history), [&first](double d) {
//       if(first) {
// 	first = false;
// 	return QString("<b>%1</b>").arg(d);
//       }
//       return QString("<i>%1</i>").arg(d);
//     });
//     d->ui->focus_analysis_history->setText(history.join("<br>"));
    QVector<QPointF> focusing_samples(history.size());
    int index=0;
    std::transform(history.begin(), history.end(), focusing_samples.begin(), [&index](double d){ return QPointF(index++, d); });
    d->focus_curve->setSamples(focusing_samples);
}


DSLR_Shooter_Window::~DSLR_Shooter_Window()
{
  d->settings.setValue("windows_settings", saveState());
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
    got_message(LogMessage::info("guider", response));
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
  got_message(LogMessage::info("General", QString("Camera connected: %1").arg(d->imager->model())));
  d->ui->camera_infos->setText(camera_infos);
  d->ui->shoot->setEnabled(true);

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
      ui->imageContainer->update();
      if(ui->enable_focus_analysis->isChecked()) {
	QMetaObject::invokeMethod(focus, "analyze", Qt::QueuedConnection, Q_ARG(QImage, ui->imageContainer->roi().isNull() ? image : image.copy(ui->imageContainer->roi())));
      } else {
	ui->focus_analysis_history->clear();
	ui->focus_analysis_value->display(0);
      }
      for(auto widget: vector<QAbstractButton*>{ui->zoomActualSize, ui->zoomFit, ui->zoomIn, ui->zoomOut})
	widget->setEnabled(!image.isNull());
      afterShot();
      long seconds_interval = *remaining > 0 ? QTime{0,0,0}.secsTo(ui->shoot_interval->time()) : 0;
      timedLambda(seconds_interval * 1000, [=]{ shoot(remaining, afterShot, afterSequence); }, q);
    });
}


void DSLR_Shooter_Window::start_shooting()
{
  long total_shots_number = d->ui->shoot_mode->currentIndex() == 0 ? 1 : d->ui->images_count->value();
  shared_ptr<long> remaining_shots = make_shared<long>(total_shots_number == 0 ? std::numeric_limits<long>::max() : total_shots_number);
  d->abort_sequence = false;
  auto setWidgetsEnabled = [=](bool enable) {
    d->ui->shoot_mode->setEnabled(enable);
    d->ui->shoot_interval->setEnabled(enable);
    d->ui->images_count->setEnabled(enable);
    d->ui->imageSettings->setEnabled(enable);
    d->ui->shoot->setEnabled(enable);
  };
  setWidgetsEnabled(false);
  d->ui->stopShooting->setVisible(total_shots_number!=1);
  d->shoot(remaining_shots, [=]{
    if(total_shots_number > 0)
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

void DSLR_Shooter_Window::got_message(const LogMessage &logMessage)
{

  qDebug() << __PRETTY_FUNCTION__ << ": " << logMessage;
  QStandardItem *when = new QStandardItem{logMessage.when.toString(Qt::DateFormat::ISODate)};
  when->setData(logMessage.when);
  QStandardItem *type = new QStandardItem{logMessage.typeDesc()};
  type->setData(logMessage.type);
  QStandardItem *from_item = new QStandardItem{logMessage.source};
  from_item->setData(logMessage.source);
  QStandardItem *message_item = new QStandardItem{logMessage.message};
  d->logs.appendRow({when, type, from_item, message_item});
  if(logMessage.type == LogMessage::Error) {
    d->trayIcon.showMessage(QString("%1 error").arg(logMessage.source), logMessage.message);
  }
}

