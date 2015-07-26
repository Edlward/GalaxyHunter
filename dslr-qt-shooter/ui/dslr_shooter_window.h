#ifndef DSLR_SHOOTER_WINDOW_H
#define DSLR_SHOOTER_WINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <imaging/imaging_driver.h>
#include "utils/dptr.h"

class LogMessage;
class LinGuider;
class QLabel;
namespace Ui {
class DSLR_Shooter_Window;
}

class DSLR_Shooter_Window : public QMainWindow
{
    Q_OBJECT

public:
    explicit DSLR_Shooter_Window(QWidget *parent = 0);
    ~DSLR_Shooter_Window();
    
public slots:
  void update_infos();

private slots:
  void on_connectLinGuider_clicked();
  void on_dither_clicked();

  void got_message(const LogMessage &logMessage);
  void camera_connected();
  void camera_disconnected();
  void start_shooting();
private slots:
  void shootModeChanged(int index);
  void focus_received(double value);
  void shoot_received(const QImage &image);
private:
  D_PTR
};

#endif // DSLR_SHOOTER_WINDOW_H
