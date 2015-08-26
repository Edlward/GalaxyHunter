#ifndef TESTINGIMAGER_H
#define TESTINGIMAGER_H
#include "imaging_driver.h"
#include <QNetworkAccessManager>
#include "commons/shootersettings.h"

class TestingImager : public Imager {
    Q_OBJECT
public:
    TestingImager(ShooterSettings &settings);
    virtual QString summary() const {
        return "---- Testing Imager ----";
    }
    virtual QString model() const {
        return "Testing Imager v.0.0";
    }
    virtual QString about() const {
        return "";
    }
    class Settings;
    friend class Settings;
    virtual std::shared_ptr<Imager::Settings> settings();
public slots:
    virtual void connect();
    virtual void disconnect();
    virtual Image::ptr shoot(const Imager::Settings::ptr &settings) const;
private:
    ShooterSettings &shooterSettings;
};
class TestingImagerDriver : public ImagingDriver
{
    Q_OBJECT
public:
    explicit TestingImagerDriver(ShooterSettings &shooterSettings, QObject *parent = 0);
protected:
    virtual void scan_imagers();
signals:
private:
  ShooterSettings &shooterSettings;

};

#endif // TESTINGIMAGER_H
