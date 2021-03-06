#include "histogram.h"
#include "ui_histogram.h"
#include "Qt/functional.h"
#include <core/settings.h>
#include "CImg/cimg_histogram.h"

using namespace std;

class Histogram::Private {
public:
  Private(Histogram *q);
  void draw_histogram();
  unique_ptr<Ui::Histogram> ui;
  unique_ptr<QCPBars> bars;
  GuLinux::Histogram<uint16_t>::ptr histogram;
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
  d->bars.reset(new QCPBars{d->ui->histogram->xAxis, d->ui->histogram->yAxis});
  d->ui->histogram->addPlottable(d->bars.get());
  auto redraw_histogram = [&] {
    Settings::instance()->setValue("histogram-bins", d->ui->bins->value());
    d->draw_histogram();
  };
  d->ui->bins->setValue(Settings::instance()->value("histogram-bins", 50).toInt() );
  connect(d->ui->bins, F_PTR(QSpinBox, valueChanged, int), redraw_histogram);
  // d->ui->histogram->addGraph();
}

void Histogram::set_image(const Image::ptr& image)
{
  d->histogram = make_shared<GuLinux::Histogram<uint16_t>>(image->cimg());
  d->draw_histogram();
}

#include <iostream>

void Histogram::Private::draw_histogram()
{
  if(!histogram)
    return;
  auto bins = histogram->bins(ui->bins->value());

  QVector<double> x(bins.size());
  QVector<double> y(bins.size());
  for(int i=0; i<bins.size(); i++) {
    x[i] = bins[i].middle;
    y[i] = bins[i].count;
  }
  bars->setWidthType(QCPBars::wtAxisRectRatio);
  bars->setWidth(1./x.size());
  bars->setData(x, y);
  ui->histogram->xAxis->setRange(0, *max_element(x.begin(), x.end()));
  ui->histogram->yAxis->setRange(0, *max_element(y.begin(), y.end()));
//   ui->histogram->yAxis->setScaleType(QCPAxis::stLogarithmic);
  ui->histogram->rescaleAxes();
  ui->histogram->replot();
}



#include "histogram.moc"