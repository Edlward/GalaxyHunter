#ifndef IMAGER_DRIVER_H
#define IMAGER_DRIVER_H

#include <QDebug>
#include <QObject>
#include <QImage>
#include <QStringList>
#include <memory>

class LogMessage;

class QImage;
class ImagingDriver;
class Imager;

typedef std::shared_ptr<ImagingDriver> ImagingDriverPtr;
typedef std::shared_ptr<Imager> ImagerPtr;

class ImagingDriver : public QObject {
    Q_OBJECT
public:
    ImagingDriver(QObject *parent = 0);
    std::vector<ImagerPtr> imagers() const { return _imagers; }
protected:
  std::vector<ImagerPtr> _imagers;
  virtual void scan_imagers() = 0;
public slots:
  void scan();
  void camera_message(Imager* camera, const QString& message);
  void camera_error(Imager *camera, const QString &message);
signals:
  void imager_message(const LogMessage&);
  void camera_connected();
  void scan_finished();
};

class ImagingDrivers : public ImagingDriver {
  Q_OBJECT
public:
  ImagingDrivers(QObject* parent = 0);

protected:
  virtual void scan_imagers();
private:
  QList<ImagingDriverPtr> imagingDrivers;
};

#include "imager.h"
#endif // IMAGER_DRIVER_H
