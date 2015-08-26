#include "dslr_shooter_window.h"
#include "imagesettingsdialog.h"
#include "messageswindow.h"
#include "camerasetup.h"
#include "commons/logmessage.h"
#include <commons/shootersettings.h>
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
#include <QResizeEvent>
#include <QSettings>
#include <QString>
#include <QtConcurrent>
#include <QComboBox>
#include <QMessageBox>
#include <QLCDNumber>
#include <QSystemTrayIcon>
#include <QInputDialog>
#include "imaging/focus.h"
#include <imaging/imagingmanager.h>
#include "qwt-src/qwt_plot_curve.h"
#include <qwt-src/qwt_plot_histogram.h>
#include <qwt-src/qwt_symbol.h>
#include "telescope/telescopecontrol.h"
#include "zoomableimage.h"

using namespace std;
using namespace std::placeholders;



class DSLR_Shooter_Window::Private {
public:
  Private(DSLR_Shooter_Window *q, Ui::DSLR_Shooter_Window *ui);
    unique_ptr<Ui::DSLR_Shooter_Window> ui;
    LinGuider *guider;
    struct LogEntry {
       QString message;
       QDateTime when;
    };

    ShooterSettings shooterSettings;
    QList<LogEntry> logEntries;
    ImagingDriverPtr imagingDriver;
    ImagerPtr imager;
    ImagingManagerPtr imagingManager;
    QSettings settings;
    
    Focus *focus;
    QwtPlotCurve *focus_curve;
    TelescopeControl *telescopeControl;
    QStandardItemModel logs;
    QSystemTrayIcon trayIcon;
    QThread* focusThread;
    CameraSetup* cameraSetup;
    void saveState();
    ZoomableImage *imageView;
private:
  DSLR_Shooter_Window *q;
};

DSLR_Shooter_Window::Private::Private(DSLR_Shooter_Window* q, Ui::DSLR_Shooter_Window* ui)
 : q(q), ui(ui), settings("GuLinux", "DSLR-Shooter"), shooterSettings{settings}, 
 imagingDriver{std::make_shared<ImagingDrivers>(shooterSettings)}, imagingManager(make_shared<ImagingManager>(shooterSettings)), trayIcon{QIcon::fromTheme("dslr-qt-shooter")}
{
}

void DSLR_Shooter_Window::closeEvent(QCloseEvent* e)
{
    QWidget::closeEvent(e);
    e->accept();
    qApp->quit();
}

void DSLR_Shooter_Window::Private::saveState()
{
  settings.setValue("windows_settings", q->saveState());
}


DSLR_Shooter_Window::DSLR_Shooter_Window(QWidget *parent) :
  QMainWindow(parent), dptr(this, new Ui::DSLR_Shooter_Window)
{
  d->trayIcon.show();
  d->logs.setHorizontalHeaderLabels({tr("Time"), tr("Type"), tr("Source"), tr("Message")});
  d->ui->setupUi(this);
  d->ui->imageContainer->setLayout(new QBoxLayout(QBoxLayout::BottomToTop));
  d->ui->imageContainer->layout()->setSpacing(0);
  d->ui->imageContainer->layout()->setMargin(0);
  d->ui->imageContainer->layout()->addWidget(d->imageView = new ZoomableImage{false, d->ui->imageContainer});
  addToolBar(d->imageView->toolbar());
  
  tabifyDockWidget(d->ui->camera_information_dock, d->ui->camera_setup_dock);
  tabifyDockWidget(d->ui->camera_information_dock, d->ui->guider_dock);
  tabifyDockWidget(d->ui->camera_information_dock, d->ui->focus_dock);
  d->ui->camera_information_dock->raise();
  
  auto logsDockWidget = new QDockWidget("Logs");
  logsDockWidget->setObjectName("logs");
  logsDockWidget->setHidden(true);
  logsDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
  logsDockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
  logsDockWidget->setWidget(new MessagesWindow{&d->logs});
  addDockWidget(Qt::BottomDockWidgetArea, logsDockWidget);
  logsDockWidget->setFloating(true);
  
  QMap<QDockWidget*, QAction*> dockWidgetsActions {
    {d->ui->camera_information_dock, d->ui->actionCamera_Information},
    {d->ui->camera_setup_dock, d->ui->actionCamera_Setup},
    {d->ui->guider_dock, d->ui->actionGuider},
    {d->ui->focus_dock, d->ui->actionFocusing},
    {logsDockWidget, d->ui->actionLogs_Messages},
  };
  
  
  d->ui->camera_setup_dock->setWidget(d->cameraSetup = new CameraSetup{d->shooterSettings});

  for(auto dockwidget : dockWidgetsActions.keys()) {
    connect(dockwidget, &QDockWidget::dockLocationChanged, [=]{ d->saveState(); });
    connect(dockwidget, &QDockWidget::topLevelChanged, [=]{ d->saveState(); });
    connect(dockwidget, &QDockWidget::visibilityChanged, [=]{ d->saveState(); });
    connect(dockwidget, &QDockWidget::visibilityChanged, [=](bool visible){ dockWidgetsActions[dockwidget]->setChecked(!dockwidget->isHidden()); });
    connect(dockWidgetsActions[dockwidget], &QAction::triggered, [=](bool checked){ dockwidget->setHidden(!checked); });
  }
  
  auto toggleWidgetsVisibility = [=](bool visible) {
    auto widgets = dockWidgetsActions.keys();
    for_each(begin(widgets), end(widgets), bind(&QWidget::setVisible, _1, visible));
  };
  connect(d->ui->actionHide_All, &QAction::triggered, bind(toggleWidgetsVisibility, false));
  connect(d->ui->actionShow_All, &QAction::triggered, bind(toggleWidgetsVisibility, true));
  
  restoreState(d->settings.value("windows_settings").toByteArray());
  
  d->telescopeControl = new TelescopeControl(this);
  QMenu *setCamera = new QMenu("Available Cameras", this);
  d->ui->actionSet_Camera->setMenu(setCamera);
  connect(d->telescopeControl, SIGNAL(message(LogMessage)), this, SLOT(got_message(LogMessage)));
  connect(d->ui->actionStop_Shooting, &QAction::triggered, [=]{ d->ui->actionStop_Shooting->setDisabled(true); d->imagingManager->abort(); });
  connect(d->ui->action_Quit, SIGNAL(triggered(bool)), qApp, SLOT(quit()));
  d->guider = new LinGuider(this);
  QTimer *updateTimer = new QTimer();
  connect(updateTimer, SIGNAL(timeout()), this, SLOT(update_infos()));
  updateTimer->start(2000);
  
  resize(QGuiApplication::primaryScreen()->availableSize() * 4 / 5);
  d->ui->actionStop_Shooting->setDisabled(true);
  
  auto set_imager = [=](const ImagerPtr &imager) {
    d->imager = imager;
    d->imagingManager->setImager(imager);
    connect(d->imager.get(), SIGNAL(connected()), this, SLOT(camera_connected()), Qt::QueuedConnection);
    connect(d->imager.get(), SIGNAL(disconnected()), this, SLOT(camera_disconnected()), Qt::QueuedConnection);
    // TODO: restore
//     connect(d->imager.get(), &Imager::exposure_remaining, this, [=](int seconds){
//       statusBar()->showMessage(tr("Exposure remaining: %1").arg(QTime(0,0,0).addSecs(seconds).toString()));
//     }, Qt::QueuedConnection);
    d->imager->connect();
  };

  connect(d->imagingDriver.get(), SIGNAL(imager_message(LogMessage)), this, SLOT(got_message(LogMessage)), Qt::QueuedConnection);
  connect(d->imagingDriver.get(), SIGNAL(camera_connected()), this, SLOT(camera_connected()), Qt::QueuedConnection);
  
  connect(d->ui->actionShoot, SIGNAL(triggered()), this, SLOT(start_shooting()));
  connect(d->ui->actionScan, SIGNAL(triggered(bool)), d->imagingDriver.get(), SLOT(scan()), Qt::QueuedConnection);
  connect(d->imagingDriver.get(), &ImagingDriver::scan_finished, this, [=]{
    setCamera->clear();
    for(auto camera: d->imagingDriver->imagers()) {
      connect(setCamera->addAction(camera->model()), &QAction::triggered, [=] {
        set_imager(camera);
      });
    }
  }, Qt::QueuedConnection);

  connect(d->ui->focusing_select_roi, &QPushButton::clicked,[=]{
    d->imageView->startSelectionMode();
    d->ui->focusing_clear_roi->setEnabled(true);
  });
  connect(d->ui->focusing_clear_roi, &QPushButton::clicked,[=]{
    d->imageView->clearROI();
    d->ui->focusing_clear_roi->setEnabled(false);
  });
  
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
  connect(autoScan, SIGNAL(timeout()), d->imagingDriver.get(), SLOT(scan()), Qt::QueuedConnection);
  
  d->focus = new Focus;
  d->focusThread = new QThread;
  d->focus->moveToThread(d->focusThread);
  d->focusThread->start();
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
      d->imageView->clearROI();
  });
  //d->ui->focusing_graph->resize(d->ui->toolBox->width(), d->ui->toolBox->width()*1.5);
  autoScan->start(1000);
  
  // Imaging Manager
  connect(d->imagingManager.get(), &ImagingManager::image, this, &DSLR_Shooter_Window::shoot_received);
  auto setWidgetsEnabled = [=](bool enable) {
    d->cameraSetup->shooting(!enable); // TODO add some kind of "busy" signal?
    d->ui->actionShoot->setEnabled(enable);
  };
  connect(d->imagingManager.get(), &ImagingManager::started, bind(setWidgetsEnabled, false));
  connect(d->imagingManager.get(), &ImagingManager::finished, bind(&QAction::setEnabled, d->ui->actionStop_Shooting, false));
  connect(d->imagingManager.get(), &ImagingManager::finished, bind(setWidgetsEnabled, true));
  connect(d->imagingManager.get(), SIGNAL(finished()), d->ui->statusbar, SLOT(clearMessage()));
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
  d->saveState();
  d->focusThread->quit();
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


void DSLR_Shooter_Window::camera_connected()
{
  d->ui->camera_infos->clear();
  QString camera_infos = QString("Model: %1\nSummary: %2")
    .arg(d->imager->model())
    .arg(d->imager->summary());
  got_message(LogMessage::info("General", QString("Camera connected: %1").arg(d->imager->model())));
  d->ui->camera_infos->setText(camera_infos);
  d->ui->actionShoot->setEnabled(true);
  d->cameraSetup->setCamera(d->imager);
}

void DSLR_Shooter_Window::shoot_received(const Image::ptr& image, int remaining)
{
  if(!image)
    return;
  
  if(d->shooterSettings.saveImage()) {
    image->save(d->shooterSettings.saveImageDirectory());
  }
  
  QImage img = *image;
  d->imageView->setImage(img);
  
  // d->ui->images_count->setValue(remaining); TODO readd a counter
  qDebug() << "Checking for dithering...";
  if(d->shooterSettings.ditherAfterEachShot() && d->guider->is_connected()) {
    got_message(LogMessage::info("main", "starting dithering"));
    qDebug() << "Dither enabled: dithering";
    d->guider->dither();
    got_message(LogMessage::info("main", "dithering command finished"));
  }
  if(d->ui->enable_focus_analysis->isChecked()) {
    QMetaObject::invokeMethod(d->focus, "analyze", Qt::QueuedConnection,
                              Q_ARG(QImage, d->imageView->roi().isNull() ? img : img.copy(d->imageView->roi())));
  } else {
    d->ui->focus_analysis_history->clear();
    d->ui->focus_analysis_value->display(0);
  }
}


void DSLR_Shooter_Window::start_shooting()
{
  // TODO
//   long total_shots_number = d->ui->actionShoot_mode->currentIndex() == 0 ? 1 : d->ui->images_count->value();
//   d->ui->actionStop_Shooting->setVisible(total_shots_number!=1);
  d->imagingManager->start();
  return; 
}

void DSLR_Shooter_Window::camera_disconnected()
{
  d->ui->actionShoot->setDisabled(true);
  disconnect(d->imageView, SLOT(setImage(const QImage &)));
  d->cameraSetup->setCamera({});
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

