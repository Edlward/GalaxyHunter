#include "dslr_shooter_window.h"
#include "ui_dslr_shooter_window.h"
#include "guider/linguider.h"
#include <QtCore/QTimer>
#include "imaging/imaging_driver.h"
#include <QDebug>
#include <QThread>
#include <QScrollBar>
#include <QScreen>
#include <QSettings>

class DSLR_Shooter_Window::Private {
public:
  Private(DSLR_Shooter_Window *q, Ui::DSLR_Shooter_Window *ui, ImagingDriver *imagingDriver) : q(q), ui(ui), imagingDriver(imagingDriver),
    settings("GuLinux", "DSLR-Shooter") {}
    std::unique_ptr<Ui::DSLR_Shooter_Window> ui;
    LinGuider *guider;
    struct LogEntry {
       QString message;
       QDateTime when;
    };

    QList<LogEntry> logEntries;
    ImagingDriver *imagingDriver;
    std::shared_ptr<Imager> imager;
    QSettings settings;
private:
  DSLR_Shooter_Window *q;
};


DSLR_Shooter_Window::DSLR_Shooter_Window(QWidget *parent) :
  QMainWindow(parent), d(new Private(this, new Ui::DSLR_Shooter_Window, ImagingDriver::imagingDriver() ))
{
  d->ui->setupUi(this);
  QMenu *setCamera = new QMenu("Available Cameras", this);
  d->ui->actionSet_Camera->setMenu(setCamera);
  d->guider = new LinGuider(this);
  QTimer *updateTimer = new QTimer();
  connect(updateTimer, SIGNAL(timeout()), this, SLOT(update_infos()));
  updateTimer->start(2000);
  QThread *imaging_thread = new QThread(this);
  d->imagingDriver->moveToThread(imaging_thread);
  imaging_thread->start();
  
  resize(QGuiApplication::primaryScreen()->availableSize() * 4 / 5);
  d->ui->imageContainer->setWidgetResizable(true);
  d->ui->splitter->setSizes({height()/10*8, height()/10*2});
  
  auto set_imager = [=](const std::shared_ptr<Imager> &imager) {
    d->imager = imager;
    connect(d->imager.get(), SIGNAL(connected()), this, SLOT(camera_connected()), Qt::QueuedConnection);
    connect(d->imager.get(), SIGNAL(disconnected()), this, SLOT(camera_disconnected()), Qt::QueuedConnection);
    d->imager->connect();
  };

  connect(d->imagingDriver, SIGNAL(imager_error(QString)), this, SLOT(got_error(QString)));
  connect(d->imagingDriver, SIGNAL(imager_message(QString)), this, SLOT(got_message(QString)));
  connect(d->imagingDriver, SIGNAL(camera_connected()), this, SLOT(camera_connected()));
  connect(d->ui->zoomIn, &QPushButton::clicked, [=] { d->ui->imageContainer->scale(1.2); });
  connect(d->ui->zoomOut, &QPushButton::clicked, [=] { d->ui->imageContainer->scale(0.8); });
  connect(d->ui->zoomActualSize, &QPushButton::clicked, [=] { d->ui->imageContainer->normalSize(); });
  connect(d->ui->zoomFit, &QPushButton::clicked, [=] { d->ui->imageContainer->fitToWindow(); });
  connect(d->ui->actionScan, SIGNAL(triggered(bool)), d->imagingDriver, SLOT(scan()), Qt::QueuedConnection);
  connect(d->imagingDriver, &ImagingDriver::scan_finished, this, [=]{
    setCamera->clear();
      qDebug() << __PRETTY_FUNCTION__ << ": cameras size: " << d->imagingDriver->imagers().size();
    for(auto camera: d->imagingDriver->imagers()) {
      connect(setCamera->addAction(camera->name()), &QAction::triggered, [=] {
        set_imager(camera);
      });
    }
  }, Qt::QueuedConnection);
}


DSLR_Shooter_Window::~DSLR_Shooter_Window()
{
}

void DSLR_Shooter_Window::update_infos()
{
  d->ui->statusbar->clearMessage();
  QString message = d->guider->is_connected() ? "Connected" : "Disconnected";
  if(d->guider->is_connected())
    message += " - " + d->guider->version();
  d->ui->statusbar->showMessage(message);
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
    std::transform(d->logEntries.begin(), d->logEntries.end(), std::back_inserter(log), [](const Private::LogEntry &e) { return QString("%1 - %2").arg(e.when.toString(Qt::ISODate)).arg(e.message); } );
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

void DSLR_Shooter_Window::camera_connected()
{
  qDebug() << __PRETTY_FUNCTION__;
  d->ui->camera_infos->clear();
  QString camera_infos = QString("Model: %1\nSummary: %2")
    .arg(d->imager->model())
    .arg(d->imager->summary());
  got_message(QString("Camera connected: %1").arg(camera_infos));
  d->ui->camera_infos->setText(camera_infos);
  d->ui->preview->setEnabled(true);
  connect(d->imager.get(), SIGNAL(preview(QImage)), d->ui->imageContainer, SLOT(setImage(QImage)));
  connect(d->imager.get(), &Imager::preview, this, [=]{
    for(auto widget: std::vector<QAbstractButton*>{d->ui->zoomActualSize, d->ui->zoomFit, d->ui->zoomIn, d->ui->zoomOut})
      widget->setEnabled(true);
  });
  connect(d->ui->preview, &QPushButton::clicked, d->imager.get(), &Imager::shootPreview, Qt::QueuedConnection);
}

void DSLR_Shooter_Window::camera_disconnected()
{
  d->ui->preview->setDisabled(true);
  disconnect(d->ui->preview, SIGNAL(clicked()));
  disconnect(d->ui->imageContainer, SLOT(setImage(const QImage &)));
}

