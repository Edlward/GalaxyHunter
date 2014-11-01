#include "dslr_shooter_window.h"
#include "ui_dslr_shooter_window.h"
#include "guider/linguider.h"
#include <QtCore/QTimer>
#include "imaging/imaging_driver.h"
#include <QDebug>
#include <QThread>

DSLR_Shooter_Window::DSLR_Shooter_Window(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::DSLR_Shooter_Window)
{
  ui->setupUi(this);
  guider = new LinGuider(this);
  QTimer *updateTimer = new QTimer();
  connect(updateTimer, SIGNAL(timeout()), this, SLOT(update_infos()));
  updateTimer->start(2000);
  QThread *imaging_thread = new QThread(this);
  imagingDriver = ImagingDriver::imagingDriver();
  imagingDriver->moveToThread(imaging_thread);
  imaging_thread->start();
  connect(imagingDriver, SIGNAL(imager_error(QString)), this, SLOT(got_error(QString)));
  connect(imagingDriver, SIGNAL(imager_message(QString)), this, SLOT(got_message(QString)));
  connect(imagingDriver, SIGNAL(camera_connected()), this, SLOT(camera_connected()));
  connect(imagingDriver, SIGNAL(camera_preview(QImage)), this, SLOT(got_preview(QImage)));
}

DSLR_Shooter_Window::~DSLR_Shooter_Window()
{
  delete ui;
}

void DSLR_Shooter_Window::update_infos()
{
  ui->statusbar->clearMessage();
  QString message = guider->is_connected() ? "Connected" : "Disconnected";
  if(guider->is_connected())
    message += " - " + guider->version();
  ui->statusbar->showMessage(message);
}


void DSLR_Shooter_Window::on_connectLinGuider_clicked()
{
    guider->connect();
    update_infos();
}

void DSLR_Shooter_Window::on_setupShoots_clicked()
{
  qDebug() << __PRETTY_FUNCTION__;
  QMetaObject::invokeMethod(imagingDriver, "findCamera", Qt::QueuedConnection);
}

void DSLR_Shooter_Window::on_startShooting_clicked()
{
  qDebug() << __PRETTY_FUNCTION__;
  QMetaObject::invokeMethod(imagingDriver, "preview", Qt::QueuedConnection);

}

void DSLR_Shooter_Window::on_dither_clicked()
{
    auto response = guider->dither();
    logEntries.prepend({response, QDateTime::currentDateTime()});
    update_log();
}

void DSLR_Shooter_Window::update_log()
{
    ui->logWindow->clear();
    QStringList log;
    std::transform(logEntries.begin(), logEntries.end(), std::back_inserter(log), [](const LogEntry &e) { return QString("%1 - %2").arg(e.when.toString(Qt::ISODate)).arg(e.message); } );
    ui->logWindow->setText(log.join("\n"));
}

void DSLR_Shooter_Window::got_error(const QString& error)
{
  if(error.isEmpty())
    return;
  logEntries.prepend({error, QDateTime::currentDateTime()});
  update_log();
}

void DSLR_Shooter_Window::got_message(const QString& message)
{
  if(message.isEmpty())
    return;
  logEntries.prepend({message, QDateTime::currentDateTime()});
  update_log();
}

void DSLR_Shooter_Window::camera_connected()
{
  qDebug() << __PRETTY_FUNCTION__;
  ui->camera_infos->clear();
  QString camera_infos = QString("Model: %1\nSummary: %2")
    .arg(imagingDriver->imager()->model())
    .arg(imagingDriver->imager()->summary());
  ui->camera_infos->setText(camera_infos);
}

void DSLR_Shooter_Window::got_preview(const QImage& image)
{
  ui->image->clear();
  ui->image->setPixmap(QPixmap::fromImage(image));
}

