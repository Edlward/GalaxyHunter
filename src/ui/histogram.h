#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QWidget>
#include "dptr.h"
#include <imaging/imager.h>

class Histogram : public QWidget
{
    Q_OBJECT
public:
    Histogram(QWidget* parent = nullptr);
    ~Histogram();
    void set_image(const Image::ptr &image);

private:
  D_PTR;
};

#endif // HISTOGRAM_H
