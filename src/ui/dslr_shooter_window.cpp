#include "dslr_shooter_window.h"
#include "imagesettingsdialog.h"
#include "messageswindow.h"
#include "camerasetup.h"
#include "sequenceswidget.h"
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
#include <qcustomplot.h>
#include "imaging/focus.h"
#include <imaging/imagingmanager.h>
#include <imaging/imagingsequence.h>
#include "telescope/telescopecontrol.h"
#include "GuLinux-Commons/Qt/zoomableimage.h"
#include <Qt/qlambdaevent.h>
#include "commons/version.h"

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
    TelescopeControl *telescopeControl;
    QStandardItemModel logs;
    QSystemTrayIcon trayIcon;
    QThread focusThread;
    CameraSetup* cameraSetup;
    void saveState();
    ZoomableImage *imageView;
    QThread imagingManagerThread;
    SequencesWidget* sequencesEditor;
private:
    DSLR_Shooter_Window *q;
};

DSLR_Shooter_Window::Private::Private(DSLR_Shooter_Window* q, Ui::DSLR_Shooter_Window* ui)
    : q(q), ui(ui), settings("GuLinux", "DSLR-Shooter"), shooterSettings {settings},
imagingDriver {std::make_shared<ImagingDrivers>()}, imagingManager(make_shared<ImagingManager>(shooterSettings)), trayIcon {QIcon::fromTheme("dslr-qt-shooter")}
{
}

void DSLR_Shooter_Window::closeEvent(QCloseEvent* e)
{
    qApp->quit();
    qDebug() << "window closed";
    QWidget::closeEvent(e);
}

void DSLR_Shooter_Window::Private::saveState()
{
    settings.setValue("windows_settings", q->saveState());
}


DSLR_Shooter_Window::DSLR_Shooter_Window(QWidget *parent) :
    QMainWindow(parent), dptr(this, new Ui::DSLR_Shooter_Window)
{
    d->ui->setupUi(this);
    connect(d->ui->actionAbout, &QAction::triggered, bind(&QMessageBox::about, this, tr("About"),
            tr("%1 version %2.\nDSLR management application for Astronomy").arg(qApp->applicationDisplayName())
            .arg(qApp->applicationVersion())));
    connect(d->ui->actionAbout_Qt, &QAction::triggered, &QApplication::aboutQt);
    setWindowIcon(QIcon::fromTheme(PROJECT_NAME));
    d->imagingManager->moveToThread(&d->imagingManagerThread);
    d->imagingManagerThread.start();
    d->trayIcon.show();
    d->logs.setHorizontalHeaderLabels( {tr("Time"), tr("Type"), tr("Source"), tr("Message")});
    setWindowTitle(PROJECT_NICE_NAME);
    d->ui->imageContainer->setLayout(new QBoxLayout(QBoxLayout::BottomToTop));
    d->ui->imageContainer->layout()->setSpacing(0);
    d->ui->imageContainer->layout()->setMargin(0);
    d->ui->imageContainer->layout()->addWidget(d->imageView = new ZoomableImage {false, d->ui->imageContainer});
    addToolBar(d->imageView->toolbar());
    connect(qApp, &QApplication::aboutToQuit, d->imagingManager.get(), bind(&ImagingManager::abort, d->imagingManager), Qt::QueuedConnection);
    connect(qApp, &QApplication::aboutToQuit, bind(&QThread::quit, &d->focusThread));
    connect(qApp, &QApplication::aboutToQuit, bind(&QThread::quit, &d->imagingManagerThread));
    connect(d->imagingManager.get(), &ImagingManager::message, d->ui->statusbar, &QStatusBar::showMessage, Qt::QueuedConnection);
    connect(d->imagingManager.get(), &ImagingManager::waitForUserAction, this, [=](const QString &sequenceName, qint64 autoAcceptSeconds) {
        auto message = [=](int seconds) {
            return tr("Step %1: click Ok to continue.\n%2") % sequenceName % (seconds > 0 ? tr("Will automatically continue after %1") % QTime {0,0,0} .addSecs(seconds).toString() : "");
        };
        QMessageBox *waitDialog = new QMessageBox(QMessageBox::Information, tr("Wait for %1") % sequenceName,message(autoAcceptSeconds),QMessageBox::Ok,this);
	waitDialog->setProperty("remaining_seconds", autoAcceptSeconds);
        connect(waitDialog, &QMessageBox::buttonClicked, waitDialog, &QMessageBox::accept);
        connect(waitDialog, &QMessageBox::accepted, d->imagingManager.get(), bind(&ImagingManager::action, d->imagingManager.get(), ImagingManager::ContinueDialogAccepted), Qt::QueuedConnection);
        if(autoAcceptSeconds > 0) {
            QTimer::singleShot(autoAcceptSeconds*1000, waitDialog, bind(&QMessageBox::accept, waitDialog));
	    QTimer *updateTimer = new QTimer(waitDialog);
	    connect(updateTimer, &QTimer::timeout, [=]{
	      waitDialog->setProperty("remaining_seconds", waitDialog->property("remaining_seconds").toLongLong()-1);
	      waitDialog->setText(message(waitDialog->property("remaining_seconds").toLongLong()));
	    } );
	    updateTimer->start(1000);
        }
        connect(waitDialog, &QMessageBox::finished, bind(&QMessageBox::deleteLater, waitDialog));
        waitDialog->show();
    }, Qt::QueuedConnection);

    tabifyDockWidget(d->ui->camera_information_dock, d->ui->camera_setup_dock);
    tabifyDockWidget(d->ui->camera_information_dock, d->ui->sequencesDock);
    tabifyDockWidget(d->ui->camera_information_dock, d->ui->guider_dock);
    tabifyDockWidget(d->ui->camera_information_dock, d->ui->focus_dock);
    tabifyDockWidget(d->ui->camera_information_dock, d->ui->histogram);
    d->ui->camera_information_dock->raise();

    auto logsDockWidget = new QDockWidget("Logs");
    logsDockWidget->setObjectName("logs");
    logsDockWidget->setHidden(true);
    logsDockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
    logsDockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
    logsDockWidget->setWidget(new MessagesWindow {&d->logs});
    addDockWidget(Qt::BottomDockWidgetArea, logsDockWidget);
    logsDockWidget->setFloating(true);

    d->ui->focusing_graph->addGraph();
    d->ui->histogram_plot->addGraph();

    QMap<QDockWidget*, QAction*> dockWidgetsActions {
        {d->ui->camera_information_dock, d->ui->actionCamera_Information},
        {d->ui->camera_setup_dock, d->ui->actionCamera_Setup},
        {d->ui->sequencesDock, d->ui->actionSequences_Editor},
        {d->ui->guider_dock, d->ui->actionGuider},
        {d->ui->focus_dock, d->ui->actionFocusing},
        {logsDockWidget, d->ui->actionLogs_Messages},
    };


    d->ui->camera_setup_dock->setWidget(d->cameraSetup = new CameraSetup {d->shooterSettings});
    d->ui->sequencesDock->setWidget(d->sequencesEditor = new SequencesWidget {d->shooterSettings});

    for(auto dockwidget : dockWidgetsActions.keys()) {
        connect(dockwidget, &QDockWidget::dockLocationChanged, [=] { d->saveState(); });
        connect(dockwidget, &QDockWidget::topLevelChanged, [=] { d->saveState(); });
        connect(dockwidget, &QDockWidget::visibilityChanged, [=] { d->saveState(); });
        connect(dockwidget, &QDockWidget::visibilityChanged, [=](bool visible) {
            dockWidgetsActions[dockwidget]->setChecked(!dockwidget->isHidden());
        });
        connect(dockWidgetsActions[dockwidget], &QAction::triggered, [=](bool checked) {
            dockwidget->setHidden(!checked);
        });
    }

    auto toggleWidgetsVisibility = [=](bool visible) {
        auto widgets = dockWidgetsActions.keys();
        for_each(begin(widgets), end(widgets), bind(&QWidget::setVisible, _1, visible));
    };
    connect(d->ui->actionHide_All, &QAction::triggered, bind(toggleWidgetsVisibility, false));
    connect(d->ui->actionShow_All, &QAction::triggered, bind(toggleWidgetsVisibility, true));

    restoreState(d->settings.value("windows_settings").toByteArray());

    QMenu *setCamera = new QMenu("Available Cameras", this);
    d->ui->actionSet_Camera->setMenu(setCamera);
#ifdef ENABLE_INDI
    d->telescopeControl = new TelescopeControl(this);
    connect(d->telescopeControl, &TelescopeControl::message, bind(&DSLR_Shooter_Window::got_message, this, _1));
#endif
    connect(d->ui->actionStop_Shooting, &QAction::triggered, bind(&QAction::setDisabled, d->ui->actionStop_Shooting, true));
    connect(d->ui->actionStop_Shooting, &QAction::triggered, [=] {d->imagingManager->abort();});
    connect(d->ui->action_Quit, &QAction::triggered, qApp, &QApplication::quit);
    d->guider = new LinGuider(this);
    QTimer *updateTimer = new QTimer();
    connect(updateTimer, &QTimer::timeout, this, &DSLR_Shooter_Window::update_infos);
    updateTimer->start(2000);

    resize(QGuiApplication::primaryScreen()->availableSize() * 4 / 5);
    d->ui->actionStop_Shooting->setDisabled(true);

    auto set_imager = [=](const ImagerPtr &imager) {
        d->imager = imager;
        d->imagingManager->setImager(imager);
        connect(d->imager.get(), &Imager::connected, this, &DSLR_Shooter_Window::camera_connected, Qt::QueuedConnection);
        connect(d->imager.get(), &Imager::disconnected, this, &DSLR_Shooter_Window::camera_disconnected, Qt::QueuedConnection);
        // TODO: restore
//     connect(d->imager.get(), &Imager::exposure_remaining, this, [=](int seconds){
//       statusBar()->showMessage(tr("Exposure remaining: %1").arg(QTime(0,0,0).addSecs(seconds).toString()));
//     }, Qt::QueuedConnection);
        d->imager->connect();
    };

    connect(d->imagingDriver.get(), &ImagingDriver::imager_message, this, bind(&DSLR_Shooter_Window::got_message, this, _1), Qt::QueuedConnection);
    connect(d->imagingDriver.get(), &ImagingDriver::camera_connected, this, &DSLR_Shooter_Window::camera_connected, Qt::QueuedConnection);

    connect(d->ui->actionShoot, &QAction::triggered, d->imagingManager.get(), [=] {
        d->imagingManager->start({{d->cameraSetup->imagingSequence()}});
    });
    connect(d->ui->actionStart_Sequences, &QAction::triggered, d->imagingManager.get(), [=] {
        d->imagingManager->start(d->sequencesEditor->sequence());
    });
    connect(d->ui->actionScan, &QAction::triggered, d->imagingDriver.get(), bind(&ImagingDriver::scan, d->imagingDriver), Qt::QueuedConnection);
    connect(d->imagingDriver.get(), &ImagingDriver::scan_finished, this, [=] {
        setCamera->clear();
        for(auto camera: d->imagingDriver->imagers()) {
            connect(setCamera->addAction(camera->info().model), &QAction::triggered, [=] {
                set_imager(camera);
            });
        }
    }, Qt::QueuedConnection);

    connect(d->ui->focusing_select_roi, &QPushButton::clicked,[=] {
        d->focus->clear_history();
        d->imageView->startSelectionMode();
        d->ui->focusing_clear_roi->setEnabled(true);
    });
    connect(d->ui->focusing_clear_roi, &QPushButton::clicked,[=] {
        d->imageView->clearROI();
        d->ui->focusing_clear_roi->setEnabled(false);
    });
#ifdef ENABLE_INDI
    connect(d->ui->actionConnectTelescope, &QAction::triggered, [=] {
        QString server = QInputDialog::getText(this, tr("Telescope"), tr("Enter telescope address (example: localhost:7624)"), QLineEdit::Normal, "localhost:7624");
        if(server != "") {
            QString address = server.split(":").first();
            int port = server.split(":").last().toInt();
            d->telescopeControl->open(address, port);
        }
    });
    connect(d->ui->action_Devices_Control_Panel, &QAction::triggered, bind(&TelescopeControl::showControlPanel, d->telescopeControl));
    connect(d->ui->actionRemote_Control, &QAction::triggered, bind(&TelescopeControl::showTelescopeRemoteControl, d->telescopeControl));
#else
    d->ui->menuTelescope->setEnabled(false);
#endif
    QTimer *autoScan = new QTimer(this);
    autoScan->setSingleShot(true);
    connect(autoScan, &QTimer::timeout, d->imagingDriver.get(), bind(&ImagingDriver::scan, d->imagingDriver), Qt::QueuedConnection);

    d->focus = new Focus;
    d->focus->moveToThread(&d->focusThread);
    d->focusThread.start();
    connect(d->focus, &Focus::focus_rate, this, bind(&DSLR_Shooter_Window::focus_received, this, _1), Qt::QueuedConnection);
    connect(d->ui->enable_focus_analysis, &QCheckBox::toggled, [=](bool checked) {
        d->ui->focus_analysis_widgets->setVisible(checked);
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
        d->ui->actionStart_Sequences->setEnabled(enable);
        d->ui->actionShoot->setEnabled(enable);
        d->ui->actionStop_Shooting->setEnabled(!enable);
        d->ui->actionPause_Shooting->setEnabled(!enable);
    };
    connect(d->imagingManager.get(), &ImagingManager::started, bind(setWidgetsEnabled, false));
    connect(d->imagingManager.get(), &ImagingManager::finished, bind(setWidgetsEnabled, true));
//   connect(d->imagingManager.get(), &ImagingManager::finished, this, bind(&QStatusBar::clearMessage, d->ui->statusbar)); // TODO: was it needed?
}

void DSLR_Shooter_Window::focus_received(double value)
{
    qDebug() << "got focus HFD: " << value;
    d->ui->focus_analysis_value->setText(QString::number(value, 'f', 5));
    auto history = d->focus->history();
    d->ui->focus_analysis_best_hdf->setText(QString::number(*std::min_element(begin(history), end(history)), 'f', 5));

    QVector<double> x, y;
    int index=0;
    for(int i=history.size()-1; i>=0 && x.size() < 10; i--) {
        x.push_front(i);
        y.push_front(history[i]);
    }
    d->ui->focusing_graph->graph()->setData(x, y);
    d->ui->focusing_graph->graph()->rescaleKeyAxis(false);
    d->ui->focusing_graph->graph()->valueAxis()->setRange(*std::min_element(begin(y), end(y))-.5, *std::max_element(begin(y), end(y)) +.5);
    d->ui->focusing_graph->replot();
}


DSLR_Shooter_Window::~DSLR_Shooter_Window()
{
    qDebug() << "closing...";
    d->saveState();
    d->imagingManagerThread.quit();
    d->focusThread.quit();
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
                           .arg(d->imager->info().model)
                           .arg(d->imager->info().summary);
    got_message(LogMessage::info("General", QString("Camera connected: %1").arg(d->imager->info().model)));
    d->ui->camera_infos->setText(camera_infos);
    d->ui->actionShoot->setEnabled(true);
    d->cameraSetup->setCamera(d->imager);
    d->sequencesEditor->setImager(d->imager);
}

void DSLR_Shooter_Window::shoot_received(const Image::ptr& image)
{
    if(!image)
        return;

    QImage img = *image;
    d->imageView->setImage(img);

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
        d->ui->focusing_graph->graph()->clearData();
        d->ui->focusing_graph->replot();
        d->ui->focus_analysis_value->clear();
    }
    auto histogram = image->histogram(256);
    //d->ui->histogram_plot->
    QVector<double> x(256);
    for(int i=0; i<256; i++)
      x[i] = i+1;
    QVector<double> y(histogram.size());
    copy(begin(histogram), end(histogram), begin(y));
    d->ui->histogram_plot->graph()->setLineStyle(QCPGraph::lsStepCenter);
    d->ui->histogram_plot->graph()->setData(x, y);
    d->ui->histogram_plot->graph()->rescaleKeyAxis(false);
    d->ui->histogram_plot->graph()->valueAxis()->setRange(*std::min_element(begin(y), end(y))-.5, *std::max_element(begin(y), end(y)) +.5);
    d->ui->histogram_plot->replot();
}


void DSLR_Shooter_Window::camera_disconnected()
{
    d->ui->actionShoot->setDisabled(true);
    disconnect(d->imageView, SLOT(setImage(const QImage &)));
    d->cameraSetup->setCamera( {});
    d->sequencesEditor->setImager( {});
}

void DSLR_Shooter_Window::got_message(const LogMessage &logMessage)
{

    qDebug() << __PRETTY_FUNCTION__ << ": " << logMessage;
    QStandardItem *when = new QStandardItem {logMessage.when.toString(Qt::DateFormat::ISODate)};
    when->setData(logMessage.when);
    QStandardItem *type = new QStandardItem {logMessage.typeDesc()};
    type->setData(logMessage.type);
    QStandardItem *from_item = new QStandardItem {logMessage.source};
    from_item->setData(logMessage.source);
    QStandardItem *message_item = new QStandardItem {logMessage.message};
    d->logs.appendRow( {when, type, from_item, message_item});
    if(logMessage.type == LogMessage::Error) {
        d->trayIcon.showMessage(QString("%1 error").arg(logMessage.source), logMessage.message);
    }
}


bool DSLR_Shooter_Window::event(QEvent* event)
{
    if(event->type() == QLambdaEvent::type) {
        qDebug() << "Running lambda event...";
        reinterpret_cast<QLambdaEvent*>(event)->run();
        return true;
    }
    return QMainWindow::event(event);
}

DSLR_Shooter_Window* DSLR_Shooter_Window::instance()
{
    static DSLR_Shooter_Window *instance = new DSLR_Shooter_Window();
    return instance;
}


