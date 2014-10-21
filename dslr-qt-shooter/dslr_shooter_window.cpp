#include "dslr_shooter_window.h"
#include "ui_dslr_shooter_window.h"
#include "linguider.h"
#include <QtCore/QTimer>

DSLR_Shooter_Window::DSLR_Shooter_Window(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::DSLR_Shooter_Window)
{
  ui->setupUi(this);
  guider = new LinGuider(this);
  connect(ui->actionDither, SIGNAL(triggered()), this, SLOT(dither()));
  QTimer *updateTimer = new QTimer();
  connect(updateTimer, SIGNAL(timeout()), this, SLOT(update_infos()));
  updateTimer->start(2000);
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

}

void DSLR_Shooter_Window::on_startShooting_clicked()
{

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
