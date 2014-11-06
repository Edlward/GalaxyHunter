#ifndef TESTINGIMAGER_H
#define TESTINGIMAGER_H
#include "imaging_driver.h"
#include <QNetworkAccessManager>

class TestingImager : public Imager {
  Q_OBJECT
public:
    virtual QString summary() const { return "---- Testing Imager ----"; }
    virtual QString model() const { return "Testing Imager v.0.0"; }
    virtual QString about() const { return ""; }
public slots:
  virtual void connect();
  virtual void disconnect();
  virtual void shootPreview();
};
class TestingImagerDriver : public ImagingDriver
{
    Q_OBJECT
public:
    explicit TestingImagerDriver(QObject *parent = 0);
public slots:
  virtual void scan();
signals:

};

#endif // TESTINGIMAGER_H
