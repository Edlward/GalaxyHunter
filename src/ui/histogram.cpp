#include "histogram.h"
#include "ui_histogram.h"
using namespace std;

class Histogram::Private {
public:
  Private(Histogram *q);
  unique_ptr<Ui::Histogram> ui;
};

Histogram::Private::Private(Histogram* q) : ui{new Ui::Histogram}
{

}


Histogram::~Histogram()
{
}

Histogram::Histogram(QWidget* parent) : QWidget(parent), dptr(this)
{
    d->ui->setupUi(this);
}

#include "histogram.moc"