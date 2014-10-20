#include "dslr_shooter_window.h"
#include "ui_dslr_shooter_window.h"
#include "linguider.h"

DSLR_Shooter_Window::DSLR_Shooter_Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DSLR_Shooter_Window)
{
    ui->setupUi(this);
    LinGuider *linguider = new LinGuider(this);
}

DSLR_Shooter_Window::~DSLR_Shooter_Window()
{
    delete ui;
}
