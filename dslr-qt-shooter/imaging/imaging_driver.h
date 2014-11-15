#ifndef IMAGER_DRIVER_H
#define IMAGER_DRIVER_H

#include <QObject>
#include <QStringList>
#include <memory>


class QImage;

class Imager : public QObject {
  Q_OBJECT
public:
    virtual QString summary() const = 0; // TODO: documentation
    virtual QString model() const = 0;  // TODO: documentation
    virtual QString about() const = 0; // TODO: documentation
    
    struct ComboSetting {
      QString current;
      QStringList available;
      operator bool() const { return !current.isEmpty() && !available.empty(); }
    };
    
    virtual ComboSetting shutterSpeed() const { return {}; }
    virtual ComboSetting imageFormat() const { return {}; }
    virtual ComboSetting iso() const { return {}; }
    
public slots:
  virtual void connect() = 0;
  virtual void disconnect() = 0;
  virtual void shoot() = 0;
signals:
  void connected();
  void disconnected();
  void message(Imager *, const QString &);
  void error(Imager *,const QString &);
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
  virtual void scan_imagers() = 0;
public slots:
  void scan();
  void camera_message(Imager* camera, const QString& message);
  void camera_error(Imager *camera, const QString &message);
signals:
  void imager_message(const QString &);
  void imager_error(const QString &);
  void camera_connected();
  void scan_finished();
};

#endif // IMAGER_DRIVER_H
