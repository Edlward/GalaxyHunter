#ifndef TESTINGIMAGER_H
#define TESTINGIMAGER_H
#include "imaging_driver.h"
#include <QNetworkAccessManager>
#include "commons/shootersettings.h"

class TestingImager : public Imager {
    Q_OBJECT
public:
    TestingImager();
    virtual Settings settings() const;
    virtual Info info() const;
public slots:
    virtual void connect();
    virtual void disconnect();
    virtual Image::ptr shoot(const Settings &settings) const;
private:
  int rand(int a, int b) const;
    QByteArray imageData;
};
class TestingImagerDriver : public ImagingDriver
{
    Q_OBJECT
public:
    explicit TestingImagerDriver(QObject *parent = 0);
protected:
    virtual void scan_imagers();
signals:
private:
};

#endif // TESTINGIMAGER_H
