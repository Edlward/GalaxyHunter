#ifndef TESTINGIMAGER_H
#define TESTINGIMAGER_H
#include "imaging_driver.h"
#include <QNetworkAccessManager>

class TestingImagerDriver : public ImagingDriver
{
    Q_OBJECT
public:
    explicit TestingImagerDriver(QObject *parent = 0);
    class TestingImager : public Imager {
    public:
        virtual QString summary() const { return "Testing Imager"; }
        virtual QString model() const { return "no model information available"; }
        virtual QString about() const { return ""; }
    };
    virtual std::shared_ptr<Imager> imager() const;
public slots:
  virtual void findCamera();
  virtual void preview();
signals:

private slots:
  void imageDownloaded(QNetworkReply *reply);
private:
  std::shared_ptr<TestingImager> _imager;
  QNetworkAccessManager *_network = nullptr;
};

#endif // TESTINGIMAGER_H
