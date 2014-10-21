#ifndef DSLR_SHOOTER_WINDOW_H
#define DSLR_SHOOTER_WINDOW_H

#include <QMainWindow>
#include <QtCore/QDateTime>

class LinGuider;

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

  void on_setupShoots_clicked();

  void on_startShooting_clicked();

  void on_dither_clicked();

  void update_log();

private:
    Ui::DSLR_Shooter_Window *ui;
    LinGuider *guider;
    struct LogEntry {
       QString message;
       QDateTime when;
    };

    QList<LogEntry> logEntries;
    QStringList log;
};

#endif // DSLR_SHOOTER_WINDOW_H
