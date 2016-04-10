#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QWidget>
#include "dptr.h"

class Histogram : public QWidget
{
    Q_OBJECT
public:
    Histogram(QWidget* parent = nullptr);
    ~Histogram();

private:
  D_PTR;
};

#endif // HISTOGRAM_H
