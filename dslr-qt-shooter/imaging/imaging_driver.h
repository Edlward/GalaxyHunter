#ifndef IMAGER_DRIVER_H
#define IMAGER_DRIVER_H

#include <QObject>
#include <memory>


class QImage;

class Imager : public QObject {
  Q_OBJECT
public:
    virtual QString summary() const = 0;
    virtual QString model() const = 0;
    virtual QString about() const = 0;
public slots:
  virtual void connect() = 0;
  virtual void disconnect() = 0;
  virtual void shootPreview() = 0;
signals:
  void connected();
  void disconnected();
  void preview(const QImage &);
};

class ImagingDriver : public QObject {
    Q_OBJECT
public:
    ImagingDriver(QObject *parent = 0);
    virtual std::vector<std::shared_ptr<Imager>> imagers() const { return _imagers; }
    static ImagingDriver *imagingDriver(QObject *parent = 0);
protected:
  std::vector<std::shared_ptr<Imager>> _imagers;
public slots:
  virtual void scan() = 0;
signals:
  void imager_message(const QString &);
  void imager_error(const QString &);
  void camera_connected();
  void scan_finished();
};

#endif // IMAGER_DRIVER_H
