#ifndef DSLR_SHOOTER_WINDOW_H
#define DSLR_SHOOTER_WINDOW_H

#include <QMainWindow>

namespace Ui {
class DSLR_Shooter_Window;
}

class DSLR_Shooter_Window : public QMainWindow
{
    Q_OBJECT

public:
    explicit DSLR_Shooter_Window(QWidget *parent = 0);
    ~DSLR_Shooter_Window();

private:
    Ui::DSLR_Shooter_Window *ui;
};

#endif // DSLR_SHOOTER_WINDOW_H
